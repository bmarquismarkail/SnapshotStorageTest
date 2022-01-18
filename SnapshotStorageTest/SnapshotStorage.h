#ifndef MEMORY_SNAPSHOT_H
#define MEMORY_SNAPSHOT_H

#include <utility> //for std::tuple
#include <vector>
#include <string_view>
#include <iostream>

using poolsizetype = std::vector< std::pair< AddressType, std::size_t>>::size_type;
using memsizetype = std::vector<DataType>::size_type;

namespace BMMQ {

	struct addressReturnData {
		bool isAddressInSnapshot;
		// info
		// a tuple that houses the pool index of the address,
		// the offset where the data resides relative to the pool's absolute offset
		// and the length from the data's position to the end of the pool's data.
		std::tuple< poolsizetype, memsizetype, memsizetype>* info;
		addressReturnData(bool retFlag, std::tuple< poolsizetype, memsizetype, memsizetype>* info);
	};


	template<typename AddressType, typename DataType>
	class SnapshotStorage {
		std::vector< std::pair< AddressType, std::size_t>> pool;
		std::vector<DataType> mem;
		addressReturnData isAddressInSnapshot(AddressType at);
	public:
		void read(DataType* stream, AddressType address, std::size_t count);
		void write(DataType* stream, AddressType address, std::size_t count);
	};

}

#include "impl.hpp"
#endif