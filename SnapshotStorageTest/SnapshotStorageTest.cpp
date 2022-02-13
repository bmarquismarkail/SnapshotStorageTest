#include <iostream>
#include <stdlib.h>

#include "memory/MemoryMap.hpp"
#include "memory/MemorySnapshot/MemorySnapshot.hpp"
#include "memory/MemorySnapshot/SnapshotStorage/SnapshotStorage.h"

using AddressType = unsigned short;
using DataType = unsigned char;

int main()
{
    //let's define a memorymap
    BMMQ::MemoryMap<AddressType, DataType> m;
    //let's define a MemorySnapshot
    BMMQ::MemorySnapshot snapstore(m);

    //allocation test: empty container
    DataType* stream = new DataType[5]{ 1, 2, 3, 4, 5 };
    snapstore.mem.write(stream, 10, 5);

    //allocation test: before prior allocation
    DataType* newStream = new DataType[5]{ 5, 4, 33, 2, 1 };
    snapstore.mem.write(newStream, 0, 5);

    //allocation test: after prior allocation, appending last entru
    DataType* newStream2 = new DataType[5]{ 6, 7, 88, 9, 10 };
    snapstore.mem.write(newStream2, 15, 5);

    //read test: read all available, zero if not
    DataType output[20];
    snapstore.mem.read(output, 0, 20);

    //overwrite test: all
    DataType* newStream3 = new DataType[20];
    for (int i = 0; i < 20; i++) {
        newStream3[i] = (i+1) *5;
    }

    snapstore.mem.write(newStream3, 0, 20);

    //read test: read all available
    snapstore.mem.read(output, 0, 20);

    for (int i = 0; i < 20; i++) {
        std::cout << (int)output[i] << '|';
    }

    //operator [] test
    std::cout << '\n';
    std::cout << (int)snapstore.mem[0] << '\n';// should be five
    std::cout << (int)snapstore.mem[20] << '\n';// should be zero, and allocation occurs on mem
    std::cout << (int)snapstore.mem[19] << '\n';// should be 100
    std::cout << (int)snapstore.mem[100] << '\n';// should be 0


    //iterator test
    for (auto i = snapstore.mem.begin(); i != snapstore.mem.end(); ++i) {
        std::cout << (int)*i << '|' ;
    }
    std::cout << std::endl;
    std::for_each(snapstore.mem.begin(), snapstore.mem.end(), [](auto& s) {std::cout << (int)s << '|'; });
    std::cout << std::endl;
    return 0;
}
