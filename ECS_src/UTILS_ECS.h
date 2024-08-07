
#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <cstdlib>
#include <iostream>


//#define __forceinline __attribute__((always_inline))
#define __forceinline inline

#define PDEBUG
#ifndef PDEBUG
#define PRINT(msg) std::cout << msg
#else
#define PRINT(msg) ((void)0)
#endif


#ifndef PDEBUG
#define PRINT_FOLD(msg) ((std::cout << msg), ...)
#else
#define PRINT_FOLD(msg) ((void)0)
#endif


#define DEBUG_PRINT_SYSTEM_SIGNATURE(has, notHas) \
    do { \
        PRINT("System: " << std::endl); \
        PRINT("    Has: "); \
        std::apply([](auto&&... args) { \
            PRINT_FOLD(args << ","); \
        }, has); \
        PRINT(std::endl); \
        PRINT(" NotHas: "); \
        std::apply([](auto&&... args) { \
            PRINT_FOLD(args << ","); \
        }, notHas); \
        PRINT(std::endl); \
    } while (0)

#define DEBUG_PRINT_TIGHT_SYSTEM(expr, groupQ) \
    do { \
        PRINT("System {Has: "); \
        std::unordered_set<size_t> hasSet, hasNot; \
        GetHasAndHasNotBits(expr, hasSet, hasNot); \
        for (auto h : hasSet) PRINT(h << ","); \
        PRINT(":: HasNOT: "); \
        for (auto h : hasNot) PRINT(h << ","); \
        PRINT("} hasSet leading group: " << groupQ->s << std::endl); \
        hasSet.clear(); hasNot.clear(); \
        GetHasAndHasNotBits(groupQ->_bitRep, hasSet, hasNot); \
        PRINT("    with EXPR {HAS: "); \
        for (auto h : hasSet) PRINT(h << ","); \
        PRINT(":: HasNOT: "); \
        for (auto h : hasNot) PRINT(h << ","); \
        PRINT("}" << std::endl); \
    } while (0)

void Test();




#endif