namespace BMMQ {

	template<typename AddressType, typename DataType>
	typename SnapshotStorage<AddressType, DataType>::proxy SnapshotStorage<AddressType, DataType>::operator[](AddressType idx) {
		maxAccessed = std::max(maxAccessed, idx);
		return proxy(this, idx);
	}

	template<typename AddressType, typename DataType>
	SnapshotStorage<AddressType, DataType>::proxy::proxy(SnapshotStorage* p, AddressType a)
		:parent(p), address(a) {
	}

	template<typename AddressType, typename DataType>
	DataType& SnapshotStorage<AddressType, DataType>::proxy::operator=(const DataType& rhs) {
		(*parent).at(address) = rhs;
		return (*parent).at(address);
	}

	template<typename AddressType, typename DataType>
	SnapshotStorage<AddressType, DataType>::proxy::operator DataType& () {
		auto p = (*parent).isAddressInSnapshot(address);
		if (p.isAddressInSnapshot) {
			return (*parent).at(address);
		}
		else return def;
	}

}