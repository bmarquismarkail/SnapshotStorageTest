#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <limits>

namespace BMMQ {

	template<typename A, typename D>
	addressReturnData<A, D>::addressReturnData(bool retFlag, std::tuple< poolsizetype<A>, memindextype<D>, memindextype<D>> info)
		: isAddressInSnapshot(retFlag), info(info) {}

	template<typename AddressType, typename DataType>
	addressReturnData<AddressType, DataType> SnapshotStorage<AddressType, DataType>::isAddressInSnapshot
	(AddressType at) {
		auto entry_range = 0;
		if (pool.empty() || at < pool.front().first)
			return addressReturnData<AddressType, DataType>(false, std::make_tuple(0, 0, 0));

		// if the last entry doesn't have it, then none will:
		if (at >= pool.back().first) {
			auto endAddress = pool.back().first + (mem.size() - pool.back().second) - 1;
			auto capacity = endAddress - at + 1;
			if (at > endAddress)
				// TODO: Return the index of the last entry, as well as the size of the vector
				//       For ease of integrating this on ::write()
				return addressReturnData<AddressType, DataType>(false, std::make_tuple(
					pool.size(),
					mem.size(),
					capacity));
			else {
				auto relofs = at - pool.back().first;
				return addressReturnData<AddressType, DataType>(true, std::make_tuple(
					pool.size() - 1,
					relofs,
					capacity));
			}

		}
		// find the pair closest to but not past at
		// the read and write functions will sort if necessary
		// so all we need to do is check adjacent elements
		auto iter_start = pool.begin();
		for (; std::next(iter_start) != std::prev(pool.end()); ++iter_start) {
			auto nextpoolentry = std::next(iter_start);
			if (at >= nextpoolentry->first)
				continue;

			entry_range =
				(std::next(iter_start) == pool.end() ? (mem.size()) : std::next(iter_start)->second)
				- iter_start->second;

			auto endAddress = iter_start->first + entry_range - 1;
			if (at > endAddress)
				// TODO: Return the index of this entry, as well as the size of the pool
				//       For ease of integrating this on ::write()
				return addressReturnData<AddressType, DataType>(false, std::make_tuple(
					std::distance(pool.begin(), iter_start),
					0,
					0));
			else break;
		}

		entry_range = std::next(iter_start)->second - iter_start->second;
		auto target_offset = at - iter_start->first;


		return addressReturnData<AddressType, DataType>((entry_range > target_offset), std::make_tuple(
			std::distance(pool.begin(), iter_start),
			(target_offset),
			entry_range - (at - iter_start->first)));
	}

	template<typename AddressType, typename DataType>
	void SnapshotStorage<AddressType, DataType>::read
	(DataType* stream, AddressType address, AddressType count) {

		AddressType maxaddress = std::numeric_limits<AddressType>::max();
		AddressType bounds = maxaddress - address;
		count = std::min(bounds, count);
		if (count == 0) return;

		DataType* streamIterator = stream;
		auto memit = mem.begin();

		AddressType index = address;
		auto p = isAddressInSnapshot(index);
		auto info = p.info;
		auto poolit = pool.begin();
		std::advance(poolit, std::get<0>(info));

		for (size_t i = 0; i < count; i++) {
			if (p.isAddressInSnapshot) {
				auto entrycap = std::get<2>(info);
				if (entrycap > count)
					entrycap = count;

				std::advance(memit, poolit->second + std::get<1>(info));
				std::for_each_n(streamIterator, entrycap, [&memit](auto& s) {s = *memit; memit++; });
				streamIterator += entrycap;
				index += entrycap;
				count -= entrycap;
			}
			else {
				auto zerocount = count;
				if (std::next(poolit) != pool.end()) {
					auto nextaddress = std::next(poolit)->first;
					zerocount = nextaddress - index;
				}
				std::for_each_n(streamIterator, zerocount, [](auto& s) {s = 0; });
				streamIterator += zerocount;
				index += zerocount;
				count -= zerocount;
			}
			p = isAddressInSnapshot(index);
		}

	}

	template<typename AddressType, typename DataType>
	void SnapshotStorage<AddressType, DataType>::write
	(DataType* stream, AddressType address, typename AddressType count) {

		AddressType maxaddress = std::numeric_limits<AddressType>::max();
		AddressType bounds = maxaddress - address;
		count = std::min(bounds, count);
		if (count == 0) return;

		auto memit = mem.begin();
		auto poolit = pool.begin();
		auto memindex = 0;
		if (mem.empty())
		{
			mem.insert(memit, stream, stream + count);
			pool.push_back(std::make_pair(address, 0));
			return;
		}

		auto p = isAddressInSnapshot(address);
		auto info = p.info;
		auto new_alloc_len = count;
		auto pool_index = std::get<0>(info);
		auto entrycap = std::get<2>(info);
		if (!p.isAddressInSnapshot) {
			memindex = std::get<1>(info);
			if (entrycap == -1) {
				std::advance(poolit, pool_index - 1);
			}
			else {
				std::advance(poolit, pool_index);
				poolit = pool.insert(poolit, std::make_pair(address, memindex));
			}
			std::for_each(std::next(poolit), pool.end(), [&new_alloc_len](auto& pe) {pe.second += new_alloc_len; });
		}
		else {
			std::advance(poolit, pool_index);
			if (count >= entrycap) {
				memindex = pool.at(pool_index).second + std::get<1>(info);
				auto endaddress = address + count - 1;
				auto address_return_data = isAddressInSnapshot(endaddress);
				auto address_return_info = address_return_data.info;
				auto delpoolit = pool.end();

				if (std::get<0>(address_return_info) + 1 != pool.size() )
					std::advance(delpoolit, std::get<0>(address_return_info) - pool.size());

				if (delpoolit == pool.end()) {
					entrycap = mem.size() - memindex;
					pool.erase(std::next(poolit), delpoolit);
				}
				else if (delpoolit != poolit) {
					entrycap = delpoolit->second + std::get<1>(address_return_info) + 1;
					if (std::next(poolit) == delpoolit) pool.erase(delpoolit);
					else pool.erase(std::next(poolit), delpoolit);
				}

				new_alloc_len -= entrycap;
			}
		}
		auto streamit = stream;
		std::advance(memit, memindex);
		mem.insert(memit, streamit, stream + new_alloc_len);
		memit = mem.begin();
		std::advance(memit, new_alloc_len);
		stream += new_alloc_len;
		std::for_each_n(memit, count - new_alloc_len, [&stream](auto& d) {d = *stream++; });
		return;
	}
}
