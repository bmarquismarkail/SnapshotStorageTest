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
		AddressType maxAccessed;
	public:
		void read(DataType* stream, AddressType address, AddressType count);
		void write(DataType* stream, AddressType address, AddressType count);
		DataType& at(AddressType idx);

		// class for reading and writing
		// This class will be used as a middleman to prevent allocation when none is needed
		
		class Proxy {
			AddressType address;
			SnapshotStorage* parent;
			static DataType def;
		public:
			Proxy(SnapshotStorage* p, AddressType a);
			DataType& operator=(const DataType& rhs);
			operator DataType&();

		};


		// returns a reference to the data being accessed
		// if there are no data in address idx, it will add the address to the container and return 0

		Proxy operator[](AddressType idx);

		//create a custom iterator
		struct iterator {
			iterator(SnapshotStorage* s);
			iterator(AddressType a, SnapshotStorage* p);
			DataType& operator*();
			DataType* operator&();

			iterator& operator++();

			iterator operator++(int);

			//special function to get end
			iterator end();

			bool operator== (const iterator& rhs);
			bool operator!= (const iterator& rhs);
		private:
			SnapshotStorage* parent;
			AddressType address;
				
		};

		iterator begin();
		iterator end();
	};

}

#include "SnapshotStorage.hpp"
#include "Proxy.hpp"
#include "Iterator.hpp"
#endif
