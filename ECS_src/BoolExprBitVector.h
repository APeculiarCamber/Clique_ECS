#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include <array>
#include <vector>
#include <unordered_set>
#include <bitset>
#include <limits>
#include <climits>
#include "UTILS_ECS.h"

#define TEMP_SIZED template <size_t N>
using namespace std;

// NOTE: This is treated as a C-style structure for clarity that it should be treated like proper data

/**
 * A pair of bit masks.
 * The first bit mask mustHave specifies the components that a system MUST HAVE (1) or MUST NOT HAVE (0).
 * The first bit mask caresAbout specifies the components that a system cares about (either they must exist or not).
 * If a component has a mask of 0 in caresAbout, then the isNotValid in mustHave is ignored.
 * @tparam ArrN The number of components divided by the bit size of size_t
 */
template <size_t ArrN>
struct BoolExprBitVector {
    array<size_t, ArrN> mustHave{};
    array<size_t, ArrN> caresAbout{};
};

template <typename HTuple, typename NHTuple, size_t N>
BoolExprBitVector<N> MakeFromTupleSpec(const HTuple& has, const NHTuple& notHas);
TEMP_SIZED BoolExprBitVector<N> MakeFromSpecVec(const vector<size_t>& has, const vector<size_t>& notHas);
TEMP_SIZED BoolExprBitVector<N> MakeFromStrictSpec(const std::array<size_t, N>& specs);
TEMP_SIZED BoolExprBitVector<N> MakeFromStrictSpec(std::array<size_t, N> specs);

TEMP_SIZED bool BitImplies(const BoolExprBitVector<N>& a, const BoolExprBitVector<N>& b);
TEMP_SIZED bool BitImpliesNot(const BoolExprBitVector<N>& a, const BoolExprBitVector<N>& b);
TEMP_SIZED bool BitEquiv(const BoolExprBitVector<N>& a, const BoolExprBitVector<N>& b);
TEMP_SIZED bool BitImplies(const std::array<size_t, N>& a, const BoolExprBitVector<N>& b);
TEMP_SIZED bool BitImpliesNot(const std::array<size_t, N>& a, const BoolExprBitVector<N>& b);
/*
 * Returns the set bits of the BoolExprBitVector for 'Cares to Have'
* WARNING: DOES NOT CLEAR THE OUT SET
*/
TEMP_SIZED void GetHasBits(const BoolExprBitVector<N>& a, unordered_set<size_t>& outSet);
/*
 * Returns the set bits of the BoolExprBitVector for 'Cares to NOT Have'
* WARNING: DOES NOT CLEAR THE OUT SET
*/
TEMP_SIZED void GetHasAndHasNotBits(const BoolExprBitVector<N>& a, unordered_set<size_t>& hasSet, unordered_set<size_t>& hasNotSet);
TEMP_SIZED std::array<size_t, N> operator|(const std::array<size_t, N>& arr1, const std::array<size_t, N>& arr2);

TEMP_SIZED void DEBUG_Print_Boolean_Expr(const BoolExprBitVector<N>& a);

#include "BoolExprBitVector.cpp"


#endif