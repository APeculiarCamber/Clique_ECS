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
public: // TODO: see about scope lowering, TODO : make sure that this unique_ptr doesn't mess up everything
    vector<unique_ptr<BoolExprTree<ArrN>>> _exprTrees;
    std::array<size_t, ArrN> _tags;

    BoolExprTreeManager() = default;
    explicit BoolExprTreeManager(std::array<size_t, ArrN>& tags) : _tags(tags) { }

    /*
    * Add a new group to as many boolean trees as can take it.
    * Distinctions are made appropriately for when orderings are BROKEN. HOPEFULLY!!!!!!
    */
    char AddGroup(const vector<size_t>& has, const vector<size_t>& hasNot, bool addNonImplicitlyIfImplicit=false, bool createNewTreeIfImplicit=false);
};

#include "BoolExprTreeManager.cpp"

#endif