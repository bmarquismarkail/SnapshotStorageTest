// SnapshotStorageTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

using AddressType = unsigned short;
using DataType = unsigned char;

#include <iostream>

#include "SnapshotStorage.h"

int main()
{
    //lets define a snapshotstorage class
    BMMQ::SnapshotStorage<AddressType, DataType> s;

    //lets write some data in it
    DataType* stream = new DataType[5] { 1, 2, 3, 4, 5, };
    s.write(stream, 10, 5);

    //allocation test: before
    DataType* newStream = new DataType[5]{ 5, 4, 33, 2, 1, };
    s.write(newStream, 0, 5);

    //allocation test: after
    DataType* newStream2 = new DataType[5]{ 6, 7, 88, 9, 10, };
    s.write(newStream2, 15, 5);



    DataType output[20];
    s.read(output, 0, 20);

    for (int i = 0; i < 20; i++) {
        std::cout << (int)output[i] << '|';
    }
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