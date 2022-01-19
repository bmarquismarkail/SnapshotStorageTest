#include <algorithm>
#include <iterator>
#include <cstdlib>

namespace BMMQ {

	addressReturnData::addressReturnData(bool retFlag, std::tuple< poolsizetype, memsizetype, memsizetype>* info)
		: isAddressInSnapshot(retFlag), info(info) {}

	template<typename AddressType, typename DataType>
	addressReturnData SnapshotStorage<AddressType, DataType>::isAddressInSnapshot
	(AddressType at) {
		if (pool.empty() || at < pool.front().first)
			return addressReturnData(false, new std::tuple<poolsizetype, memsizetype, memsizetype>(0, 0, 0));

		// if the last entry doesn't have it, then none will:
		if (at >= pool.back().first) {
			auto endAddress = pool.back().first + (mem.size() - pool.back().second);
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
		for (; std::next(iter_start) !=pool.end(); ++iter_start) {
			auto nextpoolentry = std::next(iter_start);
			if (at > nextpoolentry->first )
				continue;

			auto endAddress = iter_start->first + nextpoolentry->second - 1;
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
		auto entry_range =
			(std::next(iter_start) == std::prev(pool.end()) ? (mem.size()) : std::next(iter_start)->second)
			- iter_start->second;

		if (entry_range >= target_offset) {
			//return addressReturnData(true, &(*iter_start), (iter_start->second + target_offset) == 0);
			return addressReturnData(true, new std::tuple<poolsizetype, memsizetype, memsizetype>(
				std::distance(pool.begin(), iter_start),
				(iter_start->second + target_offset),
				entry_range - at));
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
			*streamIterator++ = p.isAddressInSnapshot ? mem[ (pool.at(std::get<0>(info)).second) + std::get<1>(info) ] : 0;
			index++;
		}
	}

	template<typename AddressType, typename DataType>
	void SnapshotStorage<AddressType, DataType>::write
	(DataType* stream, AddressType address, std::size_t count) {
		auto memit = mem.begin();
		auto streamit = stream;
		if (mem.empty())
		{
			mem.insert(memit, stream, stream + count);
			pool.push_back(std::make_pair(address, 0));
			return;
		}
		// Use case; mem not empty
		AddressType stream_end_addr;
		auto poolit = pool.begin();
		for (auto i = pool.begin(); i != pool.end(); ++i) {
			if (address < i->first) {
				// address < i->first. it goes right behind it
				poolit = i;
				std::advance(memit, i->second);
				size_t newalloclen = count;
				// we just need to check if we need to make a new pool:
				stream_end_addr = address + count;
				if (stream_end_addr >= i->first) {
					newalloclen -= i->first - address;
					auto otherpools = std::next(i);
					// now we need to check if the count is greater than any other pools:
					for (; otherpools != pool.end(); ++otherpools) {
						if (stream_end_addr < otherpools->first)
							break;
					}
					if (otherpools != pool.end()) {
						pool.erase(std::next(i), otherpools);
					}
					poolit->first = address;
				}
				else {
					poolit = pool.insert(i, std::make_pair(address, i->second));
					std::for_each(std::next(poolit), pool.end(), [&newalloclen](auto& pe) {pe.second += newalloclen; });
				}
				mem.insert(memit, streamit, stream + newalloclen);
				memit = mem.begin();
				std::advance(memit, newalloclen);
				stream += newalloclen;
				std::for_each_n(memit, count - newalloclen, [&stream](auto& d) {d = *stream++; });
				return;
			}
			// First, check if the address is in the pool, by checking the nearest address in the pool, and the data it contains
			auto next = std::next(i);
			size_t len = ((next == pool.end()) ? mem.size() : next->second) - i->second;
			if (address < (i->first + len)) {

			}

		}

	}
}
