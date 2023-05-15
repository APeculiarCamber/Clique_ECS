#pragma once
#include <array>
#include <vector>
#include <unordered_set>
#include <bitset>
#include <limits>
#include "PrintDebugger.h"

using namespace std;

template <size_t ArrN>
struct BoolExprBitVector {
	array<size_t, ArrN> _mustHave{};
	array<size_t, ArrN> _CaresAbout{};
};

template <size_t N>
bool MakeFromSpecVec(const vector<size_t>& has, const vector<size_t>& notHas, BoolExprBitVector<N>& outVec) {
	// Clear the cares bit vector
	for (size_t n = 0; n < N; n++)
		outVec._CaresAbout.at(n) = 0;

	// Add the NEEDS/HAS
	for (size_t h : has) {
		size_t block = h / (sizeof(size_t) * CHAR_BIT);
		size_t shift = (h - (block * (sizeof(size_t) * CHAR_BIT)));
		size_t bit = (size_t)1 << shift;

		// bool alreadyNoted = (outVec._CaresAbout.at(block) & bit) != 0;
		outVec._CaresAbout.at(block) |= bit;
		outVec._mustHave.at(block) |= bit; // add the bit
	}

	// Add the NEEDS NOT/HAS NOT
	for (size_t h : notHas) {
		size_t block = h / (sizeof(size_t) * CHAR_BIT);
		size_t shift = (h - (block * (sizeof(size_t) * CHAR_BIT)));
		size_t bit = (size_t)1 << shift;

		// bool alreadyNoted = (outVec._CaresAbout.at(block) & bit) != 0;
		outVec._CaresAbout.at(block) |= bit;
		outVec._mustHave.at(block) &= ~bit; // remove the bit
	}

	return true;
}

// TODO : repair function, make work good...
template <typename HTuple, typename NHTuple, size_t N>
BoolExprBitVector<N> MakeFromTupleSpec(const HTuple& has, const NHTuple& notHas) {
	BoolExprBitVector<N> expr{};
	PRINT("Starting with expr for TupleSpec:   ");
	DEBUG_Print_Boolean_Expr(expr);
	std::apply([&](auto... h) {
		((expr._CaresAbout.at(h / (sizeof(size_t) * CHAR_BIT)) |= ((size_t)1 << (h % (sizeof(size_t) * CHAR_BIT)))), ...);
		((expr._mustHave.at(h / (sizeof(size_t) * CHAR_BIT)) |= ((size_t)1 << (h % (sizeof(size_t) * CHAR_BIT)))), ...);
		}, has);

	std::apply([&](auto... h) {
		((expr._CaresAbout.at(h / (sizeof(size_t) * CHAR_BIT)) |= ((size_t)1 << (h % (sizeof(size_t) * CHAR_BIT)))), ...);
		// ((expr._mustHave.at(h / (sizeof(size_t) * CHAR_BIT)) &= ~((size_t)1 << (h % (sizeof(size_t) * CHAR_BIT)))), ...);
		}, notHas);

	PRINT("Made this expr with TupleSpec:   ");
	DEBUG_Print_Boolean_Expr(expr);
	return expr;
}


template <size_t N>
BoolExprBitVector<N> MakeFromStrictSpec(const std::array<size_t, N>& specs) {
	std::array<std::size_t, N> allCare;
	std::fill(allCare.begin(), allCare.end(), std::numeric_limits<std::size_t>::max());
	return { specs, allCare };
}

template <size_t N>
BoolExprBitVector<N> MakeFromStrictSpec(std::array<size_t, N> specs) {
	std::array<std::size_t, N> allCare;
	std::fill(allCare.begin(), allCare.end(), std::numeric_limits<std::size_t>::max());
	return { specs, allCare };
}


template <size_t N>
bool BitImplies(const BoolExprBitVector<N>& a, const BoolExprBitVector<N>& b) {
	// a implies b when, 
		// a.has is 1 and CARES for all b.has is 1,
		// a.has is 0 and CARES for all b.has is 0 AND b CARES.
	bool implies = true;
	for (size_t n = 0; n < N; n++) {
		// bit vec with same bits as a when a cares, and OPPOSITE as b when a doesn't care
		size_t adversary_a = (a._mustHave.at(n) & a._CaresAbout.at(n)) | (~b._mustHave.at(n) & ~a._CaresAbout.at(n));
		// not(xor) : bitwise equals
		size_t matches = ~(adversary_a ^ b._mustHave.at(n));
		// if b does not care, set it to an automatic match
		matches |= ~b._CaresAbout.at(n);
		// All elements need to 'match'
		implies &= ((~matches) == 0);
	}
	return implies;
}

template <size_t N> 
bool BitImpliesNot(const BoolExprBitVector<N>& a, const BoolExprBitVector<N>& b) {
	// a implies ~b when,
		// a.has is 1 and CARES for ANY b.has 0 and CARES
		// a.has is 0 and CARES for ANY b.has 1 and CARES
	bool impliesNot = false;
	for (size_t n = 0; n < N; n++) {
		// Care vector
		size_t cares = a._CaresAbout.at(n) & b._CaresAbout.at(n);
		// 1 if bits NOT equal
		size_t matches = a._mustHave.at(n) ^ b._mustHave.at(n);
		// only accept 1s from cared about regions
		matches &= cares;
		// Only 1 element needs to have a 1
		impliesNot |= (matches != 0);
	}
	return impliesNot;
}

template <size_t N>
bool BitEquiv(const BoolExprBitVector<N>& a, const BoolExprBitVector<N>& b) {
	return BitImplies(a, b) && BitImplies(b, a);
}



template <size_t N>
bool BitImplies(const std::array<size_t, N>& a, const BoolExprBitVector<N>& b) {
	// a implies b when, 
		// a.has is 1 and CARES for all b.has is 1,
		// a.has is 0 and CARES for all b.has is 0 AND b CARES.
	bool implies = true;
	for (size_t n = 0; n < N; n++) {
		// bit vec with same bits as a when a cares, and OPPOSITE as b when a doesn't care
		size_t adversary_a = (a.at(n)) /* | (~b._mustHave.at(n) & ~a._CaresAbout.at(n)) */;
		// not(xor) : bitwise equals
		size_t matches = ~(adversary_a ^ b._mustHave.at(n));
		// if b does not care, set it to an automatic match
		matches |= ~b._CaresAbout.at(n);
		// All elements need to 'match'
		implies &= ((~matches) == 0);
	}
	return implies;
}

template <size_t N>
void DEBUG_Print_Boolean_Expr(const BoolExprBitVector<N>& a) {
	std::unordered_set<size_t> has, hasNot;
	GetHasAndHasNotBits(a, has, hasNot);
	PRINT("Expr: {has: (");
	for (auto h : has) PRINT(h << ",");
	PRINT(" ), hasnot: (");
	for (auto h : hasNot) PRINT(h << ",");
	PRINT(") }" << std::endl);
}

template <size_t N>
void FORCE_DEBUG_Print_Boolean_Expr(const BoolExprBitVector<N>& a) {
	std::unordered_set<size_t> has, hasNot;
	GetHasAndHasNotBits(a, has, hasNot);
	std::cout << "Expr: {has: (";
	for (auto h : has) std::cout << h << ",";
	std::cout << " ), hasnot: (";
	for (auto h : hasNot) std::cout << h << ",";
	std::cout << ") }" << std::endl;
}

template <size_t N>
bool BitImpliesNot(const std::array<size_t, N>& a, const BoolExprBitVector<N>& b) {
	// a implies ~b when,
		// a.has is 1 and CARES for ANY b.has 0 and CARES
		// a.has is 0 and CARES for ANY b.has 1 and CARES
	bool impliesNot = false;
	for (size_t n = 0; n < N; n++) {
		// Care vector
		size_t cares = b._CaresAbout.at(n);
		// 1 if bits NOT equal
		size_t matches = a.at(n) ^ b._mustHave.at(n);
		// only accept 1s from cared about regions
		matches &= cares;
		// Only 1 element needs to have a 1
		impliesNot |= (matches != 0);
	}
	return impliesNot;
}

/* 
* DOES NOT CLEAR THE OUT SET
* 
*/ 
template <size_t N>
void GetHasBits(const BoolExprBitVector<N>& a, unordered_set<size_t>& outSet) {
	for (size_t n = 0; n < N; n++) {
		size_t has = a._CaresAbout.at(n) & a._mustHave.at(n);
		size_t currentShift = 0;
		while (has != 0) {
			if ((has & 1) != 0) outSet.insert(currentShift + (n * (sizeof(size_t) * CHAR_BIT)));

			has = has >> 1;
			currentShift += 1;
		}
	}
}

template <size_t N>
void GetHasAndHasNotBits(const BoolExprBitVector<N>& a, unordered_set<size_t>& hasSet, unordered_set<size_t>& hasNotSet) {
	for (size_t n = 0; n < N; n++) {
		size_t cares = a._CaresAbout.at(n);
		size_t has = a._mustHave.at(n);
		size_t currentShift = 0;
		while (cares != 0) {
			if ((cares & 1) != 0) {
				if ((has & 1) != 0)
					hasSet.insert(currentShift + (n * (sizeof(size_t) * CHAR_BIT)));
				else
					hasNotSet.insert(currentShift + (n * (sizeof(size_t) * CHAR_BIT)));
			}
			has = has >> 1;
			cares = cares >> 1;
			currentShift += 1;
		}
	}
}

template <size_t N>
void GetHasBits(const std::array<size_t, N>& a, unordered_set<size_t>& hasSet) {
	for (size_t n = 0; n < N; n++) {
		size_t has = a.at(n);
		size_t currentShift = 0;
		while (has != 0) {
			if ((has & 1) != 0)
				hasSet.insert(currentShift + (n * (sizeof(size_t) * CHAR_BIT)));

			has = has >> 1;
			currentShift += 1;
		}
	}
}





template <size_t N>
std::array<size_t, N> operator|(const std::array<size_t, N>& arr1, const std::array<size_t, N>& arr2) {
	std::array<size_t, N> result{};
	for (size_t i = 0; i < N; i++) {
		result[i] = arr1[i] | arr2[i];
	}
	return result;
}