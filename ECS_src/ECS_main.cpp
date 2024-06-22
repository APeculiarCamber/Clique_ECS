// ECS_main.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <ctime>
#include <algorithm>
#include "SystemImpl.h"
#include "CoreComponentManagerImpl.h"
#include "ECS_Manager.h"
#include "RateChangeBenchmark.h"
#include "ExampleECS.h"

// #define RUN_DERIV_BENCH

int main()
{
#ifndef RUN_DERIV_BENCH
    ExampleECS();
#else
    // RunTagsBasedBenchmark(100, 100, std::cout);
    // RunPartialTestings();
    
    // Add group A, B
    // Want to add group C & B
        // We can make a partial using C rooted, then going RIGHT with C & B
        // This would require some adjuctments to traversal
        // 
        // The general would be to create a new group which uses only the groups that don't already have a group tree

    // Open the file "output.txt" for writing
    // std::ofstream outFile("change_test_output.txt");
    std::ofstream outFile("change_test_output_supl.txt");

    // Check if the file was opened successfully
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return 1;
    }

    // NOTE : also keep in mind: sparse array vs hash, tup vs no tup... + counting install times

    constexpr int inplace = 0, shuffle = 1, sort = 2;
    const vector<const char*> toStr { "inplace", "shuffle", "sort" };
    outFile << "{";
    for (size_t numItrs : {1000})
        for (size_t numEntities : {10000, 50000, 100000, 500000, 1000000, 5000000 })
            for (size_t numDerivs : {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}) {
                for (int initialOrder : {inplace, shuffle, sort}) {
                    outFile << "(" << numEntities << ", " << numDerivs << ", '" << toStr[initialOrder] << "'): ";
                    Rate::RunChangeTimeBenchmark(numEntities, numItrs, numDerivs, true,
                                                 initialOrder == shuffle, initialOrder == sort, outFile);
                }
            }
    outFile << "}" << std::endl;
    outFile.close();
#endif
}
