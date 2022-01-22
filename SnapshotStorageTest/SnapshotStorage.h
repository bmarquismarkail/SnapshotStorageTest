#ifndef MEMORY_SNAPSHOT_H
#define MEMORY_SNAPSHOT_H

#include <utility> //for std::tuple
#include <vector>
#include <string_view>
#include <iostream>

template<typename T>
using poolsizetype = typename std::vector< std::pair< T, std::size_t>>::size_type;

template<typename T>
using memindextype = typename std::vector<T>::difference_type;

namespace BMMQ {

	template<typename A, typename D>
	struct addressReturnData {
		bool isAddressInSnapshot;
		// info
		// a tuple that houses:
		// 1, the pool index of the address,
		// 2, the offset where the data resides relative to the pool's absolute offset
		// and 3, the length from the data's position to the end of the pool's data.
		std::tuple< poolsizetype<A>, memindextype<D>, memindextype<D>> info;
		addressReturnData(bool retFlag, std::tuple< poolsizetype<A>, memindextype<D>, memindextype<D>> info);
	};


	template<typename AddressType, typename DataType>
	class SnapshotStorage {
		std::vector< std::pair< AddressType, std::size_t>> pool;
		std::vector<DataType> mem;
		addressReturnData<AddressType, DataType> isAddressInSnapshot(AddressType at);
	public:
		void read(DataType* stream, AddressType address, std::size_t count);
		void write(DataType* stream, AddressType address, std::size_t count);
	};

}

#include "impl.hpp"
#endif
