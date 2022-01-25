// SnapshotStorageTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <stdlib.h>

#include "SnapshotStorage.h"

using AddressType = unsigned short;
using DataType = unsigned char;

int main()
{
    //lets define a snapshotstorage class
    BMMQ::SnapshotStorage<AddressType, DataType> s;

    //allocation test: empty container
    DataType* stream = new DataType[5]{ 1, 2, 3, 4, 5 };
    s.write(stream, 10, 5);

    //allocation test: before prior allocation
    DataType* newStream = new DataType[5]{ 5, 4, 33, 2, 1 };
    s.write(newStream, 0, 5);

    //allocation test: after prior allocation, appending last entru
    DataType* newStream2 = new DataType[5]{ 6, 7, 88, 9, 10 };
    s.write(newStream2, 15, 5);

    //read test: read all available, zero if not
    DataType output[20];
    s.read(output, 0, 20);

    //overwrite test: all
    DataType* newStream3 = new DataType[20];
    for (int i = 0; i < 20; i++) {
        newStream3[i] = (i+1) *5;
    }

    s.write(newStream3, 0, 20);

    //read test: read all available
    s.read(output, 0, 20);

    for (int i = 0; i < 20; i++) {
        std::cout << (int)output[i] << '|';
    }

    //operator [] test
    std::cout << '\n';
    std::cout << (int)s[0] << '\n';// should be five
    std::cout << (int)s[20] << '\n';// should be zero, and allocation occurs on mem
    std::cout << (int)s[19] << '\n';// should be 100
    std::cout << (int)s[100] << '\n';// should be zero, and a new pool entry is allocated as well as as memory


    //iterator test
    for (auto i = s.begin(); i != s.end(); ++i) {
        std::cout << (int)*i << '|' ;
    }
    std::cout << std::endl;
    std::for_each(s.begin(), s.end(), [](auto& s) {std::cout << (int)s << '|'; });
    std::cout << std::endl;
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
