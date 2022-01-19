#include <algorithm>
#include <iterator>
#include <cstdlib>

namespace BMMQ {

	addressReturnData::addressReturnData(bool retFlag, std::tuple< poolsizetype, memsizetype, memsizetype>* info)
		: isAddressInSnapshot(retFlag), info(info) {}

	template<typename AddressType, typename DataType>
	addressReturnData SnapshotStorage<AddressType, DataType>::isAddressInSnapshot
	(AddressType at) {
		auto entry_range = 0;
		if (pool.empty() || at < pool.front().first)
			return addressReturnData(false, new std::tuple<poolsizetype, memsizetype, memsizetype>(0, 0, 0));

		// if the last entry doesn't have it, then none will:
		if (at >= pool.back().first) {
			auto endAddress = pool.back().first + (mem.size() - pool.back().second) - 1;
			if (at > endAddress)
				// TODO: Return the index of the last entry, as well as the size of the vector
				//       For ease of integrating this on ::write()
				return addressReturnData(false, new std::tuple<poolsizetype, memsizetype, memsizetype>(
					pool.size(),
					mem.size(),
					0));
			else {
				auto relofs = at - pool.back().first;
				auto capacity = endAddress - at;
				return addressReturnData(true, new std::tuple<poolsizetype, memsizetype, memsizetype>(
					pool.size() - 1,
					relofs,
					capacity));
			}

		}
		// find the pair closest to but not past at
		// the read and write functions will sort if necessary
		// so all we need to do is check adjacent elements
		auto iter_start = pool.begin();
		for (; std::next(iter_start) != pool.end(); ++iter_start) {
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
				return addressReturnData(false, new std::tuple<poolsizetype, memsizetype, memsizetype>(
					std::distance(pool.begin(), iter_start),
					0,
					0));
			else break;
		}

		auto target_offset = at - iter_start->first;

		if (entry_range >= target_offset) {
			//return addressReturnData(true, &(*iter_start), (iter_start->second + target_offset) == 0);
			return addressReturnData(true, new std::tuple<poolsizetype, memsizetype, memsizetype>(
				std::distance(pool.begin(), iter_start),
				(target_offset),
				entry_range - (at - iter_start->first) ));
		}
		return addressReturnData(false, new std::tuple<poolsizetype, memsizetype, memsizetype>(0, 0, 0));
	}

	template<typename AddressType, typename DataType>
	void SnapshotStorage<AddressType, DataType>::read
	(DataType* stream, AddressType address, std::size_t count) {
		DataType* streamIterator = stream;
		AddressType index = address;
		for (size_t i = 0; i < count; i++) {
			auto p = isAddressInSnapshot(index);
			auto info = *(p.info);
			*streamIterator++ = p.isAddressInSnapshot ? mem[(pool.at(std::get<0>(info)).second) + std::get<1>(info)] : 0;
			index++;
		}
	}

	template<typename AddressType, typename DataType>
	void SnapshotStorage<AddressType, DataType>::write
	(DataType* stream, AddressType address, std::size_t count) {
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
		auto info = *(p.info);
		auto new_alloc_len = count;
		//auto pool_index = (pool.at(std::get<0>(info)).second);
		auto pool_index = std::get<0>(info);
		auto entrycap = std::get<2>(info);
		if (!p.isAddressInSnapshot) {
			memindex = std::get<1>(info);
			std::advance(poolit, pool_index);
			poolit = pool.insert(poolit, std::make_pair(address, memindex));
			std::for_each(std::next(poolit), pool.end(), [&new_alloc_len](auto& pe) {pe.second += new_alloc_len; });
		}
		else {
			if (count >= entrycap) {
				memindex = pool.at(pool_index).second + std::get<1>(info);
				new_alloc_len = count - entrycap;
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