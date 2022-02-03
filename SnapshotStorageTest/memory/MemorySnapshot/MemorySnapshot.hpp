#ifndef MEMORY_SNAPSHOT_H
#define MEMORY_SNAPSHOT_H


#include "SnapshotStorage/SnapshotStorage.h"

namespace BMMQ {

	template<typename A, typename D>
	class MemorySnapshot {
		SnapshotStorage<A,D> mjj;
	};
}
#include "templ/MemorySnapshot.impl.hpp"
#endif