
namespace BMMQ {
	template<typename A, typename D>
	MemorySnapshot<A, D>::MemorySnapshot(MemoryMap<A,D> &m) 
	:mem(m) {}
}