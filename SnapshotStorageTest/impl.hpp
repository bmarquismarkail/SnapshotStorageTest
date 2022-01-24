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
		auto endAddress = 0;
		if (pool.empty())
			return addressReturnData<AddressType, DataType>(false, std::make_tuple(0, 0, 0));

		if (at < pool.front().first) {
			return addressReturnData<AddressType, DataType>(false, std::make_tuple(0, 0, at - pool.front().first ));
		}

		// if the last entry doesn't have it, then none will:
		if (at >= pool.back().first) {

			auto capacity = (mem.size() - pool.back().second);
			endAddress = pool.back().first +  capacity - 1;
			if (at > endAddress)
				// TODO: Return the index of the last entry, as well as the size of the vector
				//       For ease of integrating this on ::write()
				return addressReturnData<AddressType, DataType>(false, std::make_tuple(
					pool.size() - 1,
					mem.size(),
					at - endAddress ));
			else {
				auto relofs = at - pool.back().first;
				return addressReturnData<AddressType, DataType>(true, std::make_tuple(
					pool.size() - 1,
					relofs,
					capacity - relofs));
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

			endAddress = iter_start->first + entry_range - 1;
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
		endAddress = iter_start->first + entry_range - 1;
		auto target_offset = at - iter_start->first;

		return addressReturnData<AddressType, DataType>((at < endAddress), std::make_tuple(
			std::distance(pool.begin(), iter_start),
			(target_offset),
			entry_range - target_offset ));
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
		auto poolit = pool.begin();
		std::advance(poolit, std::get<0>(p.info));

		for (size_t i = 0; i < count; i++) {
			if (p.isAddressInSnapshot) {
				auto entrycap = std::get<2>(p.info);
				if (entrycap > count)
					entrycap = count;

				std::advance(memit, poolit->second + std::get<1>(p.info));
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
			std::advance(poolit, pool_index);
			if (std::abs(entrycap) != 1 ) {
				poolit = pool.insert(( (entrycap < 0) ? poolit : std::next(poolit)), std::make_pair(address, memindex));
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

				if (std::get<0>(address_return_info) + 1 != pool.size())
					std::advance(delpoolit, std::get<0>(address_return_info) - pool.size() + 1);

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

	// because the [] operator may be used for write access, idx might need to be allocated
	template<typename AddressType, typename DataType>
	DataType& SnapshotStorage<AddressType, DataType>::operator[](AddressType idx) {
		auto p = isAddressInSnapshot(idx);
		if (p.isAddressInSnapshot)
			return mem[pool.at(std::get<0>(p.info)).second + std::get<1>(p.info)];

		auto poolit = pool.begin();
		auto memindex = std::get<1>(p.info);
		auto pool_index = std::get<0>(p.info);
		std::advance(poolit, pool_index);
		if (std::abs(std::get<2>(p.info)) != 1) {
			poolit = pool.insert(((std::get<2>(p.info) < 0) ? poolit : std::next(poolit)), std::make_pair(idx, memindex));
		}
		if (std::next(poolit) != pool.end() && std::get<1>(p.info) + std::get<2>(p.info) == std::next(poolit)->first) {
			pool.erase(std::next(poolit));
		}
		std::for_each(std::next(poolit), pool.end(), [](auto& pe) {pe.second++; });
		auto memit = mem.begin();
		std::advance(memit, memindex);
		memit = mem.insert(memit, 0);
		return *memit;
	}

	template<typename AddressType, typename DataType>
	SnapshotStorage<AddressType, DataType>::iterator::iterator(SnapshotStorage* p)
		:parent(p) {
		address = 0;
	}

	template<typename AddressType, typename DataType>
	SnapshotStorage<AddressType, DataType>::iterator::iterator(AddressType a, SnapshotStorage* p)
		:address(a), parent(p) {
	}

	template<typename AddressType, typename DataType>
	DataType& SnapshotStorage<AddressType, DataType>::iterator::operator*() { return (*parent)[address]; }

	template<typename AddressType, typename DataType>
	DataType* SnapshotStorage<AddressType, DataType>::iterator::operator&() { return &(*parent)[address]; }

	template<typename AddressType, typename DataType>
	typename SnapshotStorage<AddressType, DataType>::iterator& SnapshotStorage<AddressType, DataType>::iterator::operator++() {
		address++;
		return *this;
	}

	template<typename AddressType, typename DataType>
	typename SnapshotStorage<AddressType, DataType>::iterator SnapshotStorage<AddressType, DataType>::iterator::operator++(int) {
		iterator tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename AddressType, typename DataType>
	typename SnapshotStorage<AddressType, DataType>::iterator SnapshotStorage<AddressType, DataType>::iterator::end() {
		auto lastAddress = parent->pool.back().first + (parent->mem.size() - parent->pool.back().second);
		address = lastAddress;
		return *this;
	}

	template<typename AddressType, typename DataType>
	bool SnapshotStorage<AddressType, DataType>::iterator::operator== (const typename SnapshotStorage<AddressType, DataType>::iterator& rhs) {
		return (address == rhs.address);
	}

	template<typename AddressType, typename DataType>
	bool SnapshotStorage<AddressType, DataType>::iterator::operator!= (const typename SnapshotStorage<AddressType, DataType>::iterator& rhs) {
		return (address != rhs.address);
	}

	template<typename AddressType, typename DataType>
	typename SnapshotStorage<AddressType, DataType>::iterator SnapshotStorage<AddressType, DataType>::begin() {
		return iterator(0, this);
	}

	template<typename AddressType, typename DataType>
	typename SnapshotStorage<AddressType, DataType>::iterator SnapshotStorage<AddressType, DataType>::end() {
		iterator endit(this);
		return endit.end();
	}
}
