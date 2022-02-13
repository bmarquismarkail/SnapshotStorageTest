#ifndef MEMORY_SNAPSHOT_H
#define MEMORY_SNAPSHOT_H


#include "SnapshotStorage/SnapshotStorage.h"
#include "../MemoryMap.hpp"

namespace BMMQ {

	template<typename A, typename D>
	class MemorySnapshot {
	public:
		MemorySnapshot(MemoryMap<A,D> &m);
		//void copyRegisterFromMainFile(std::string_view regId, RegisterFile<AddressType>& from);
		//CPU_Register<RegType>* findOrCreateNewRegister(const std::string& regId, bool isPair = false);
		//RegisterFile<RegType> file;
		SnapshotStorage<A, D> mem;
	};
}
#include "templ/MemorySnapshot.impl.hpp"
#endif