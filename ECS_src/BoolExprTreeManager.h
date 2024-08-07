#ifndef TREE_MANAGER_H
#define TREE_MANAGER_H

#include <vector>
#include <unordered_set>
#include <array>
#include <iostream>
#include <memory>
#include "BoolExprBitVector.h"
#include "BooleanExprTree.h"

template <size_t ArrN>
class BoolExprTreeManager {
protected:
    vector<unique_ptr<BoolExprTree<ArrN>>> m_exprTrees;
    std::array<size_t, ArrN> m_tags;

public:
    BoolExprTreeManager() = default;
    explicit BoolExprTreeManager(std::array<size_t, ArrN>& tags) : m_tags(tags) { }

    const vector<unique_ptr<BoolExprTree<ArrN>>>& GetExpressionTrees() const { return m_exprTrees; }
    const std::array<size_t, ArrN>& GetTagBits() const { return m_tags; }
    /*
    * Add a new group to as many boolean trees as can take it.
    * Distinctions are made appropriately for when orderings are BROKEN. HOPEFULLY!!!!!!
    */
    char AddGroup(const vector<size_t>& has, const vector<size_t>& hasNot, bool addNonImplicitlyIfImplicit=false, bool createNewTreeIfImplicit=false);
};

#include "BoolExprTreeManager.cpp"

#endif