#pragma once
#include <array>
#include <vector>
#include <unordered_set>
#include <bitset>
#include <cassert>
#include <memory>
#include "UTILS_ECS.h"
#include "BoolExprBitVector.h" // TOPORT

#define EXPLICIT_ADD 1
#define IMPLICIT_ADD 2
#define FAILURE_ADD 3

using namespace std;

template <size_t ArrN>
class BoolExprNode {
public:
    BoolExprNode<ArrN>* _parent = nullptr;
    unique_ptr<BoolExprNode<ArrN>> _left = nullptr;
    unique_ptr<BoolExprNode<ArrN>> _right = nullptr;
    bool _explicitRep = false, _hasBitRep = false;
    BoolExprBitVector<ArrN> _bitRep;

    explicit BoolExprNode(BoolExprNode<ArrN>* parent) : _parent(parent), _explicitRep(false), _hasBitRep(false), _bitRep{} {}
    BoolExprNode(BoolExprNode<ArrN>* parent, BoolExprBitVector<ArrN>& bitRep, bool explRep=true)
        : _parent(parent), _explicitRep(parent == nullptr || (parent->_explicitRep) && explRep),
       _bitRep{ ._mustHave = bitRep._mustHave, ._CaresAbout = bitRep._CaresAbout }, _hasBitRep(true) {}

    /**
    * Collect all the component types which are included as ONs in the far left side EXPLICIT nodes
    */
    void PropogateExplicitComponents(unordered_set<size_t>& explicitComponents, const std::array<size_t, ArrN>& tags = {});

    /**
    * Returns true if the node contains the bit representation
    */
    bool ContainsExpr(BoolExprBitVector<ArrN>& bitRep);

    /**
    * Returns true if the node contains the bit representation
    */
    bool ContainsExpr_AssumeParent(BoolExprBitVector<ArrN>& bitRep);

    /**
    *   TODO : currently only adds to leaves, which is manageable but NOT ideal
    *   Assumes TREE will handle a re-root, so it DOES NOT
    *	The main assumption this allows is parent has already done ContainedBy checks on its kids
    */
    BoolExprNode<ArrN>* AddExpr(BoolExprBitVector<ArrN>& bitRep);


    /**
    *   Assumes TREE will handle a re-root, so it DOES NOT
    *	The main assumption this allows is parent has already done ContainedBy checks on its kids
    *
    *   This method inserts the bit rep, if it can and GUARENTEES that the
    */
    BoolExprNode<ArrN>* AddExprImplicit(BoolExprBitVector<ArrN>& bitRep, bool onFarLeft = true);


    /**
    * TODO: these functions do not currently support IsContained by, since AddExpr also doesn't yet
    */
    bool CanAdd(BoolExprBitVector<ArrN>& bitRep);

    bool CanBeAddedAsExplicit(BoolExprBitVector<ArrN>& bitRep);
};

template <size_t ArrN>
class BoolExprTree {
public:
    // list of explicit components
    // root
    unordered_set<size_t> _explicitAffectedComponents;
    unique_ptr<BoolExprNode<ArrN>> _root = nullptr;

    bool CouldBeAdded(BoolExprBitVector<ArrN>& a);

    // bool WillBeAddedAsExplicit(BoolExprBitVector<ArrN>& a);

    bool hasAsExplicitComponent(size_t comp) {
        return _explicitAffectedComponents.find(comp) != _explicitAffectedComponents.end();
    }

    char Add(BoolExprBitVector<ArrN>& a, const std::array<size_t, ArrN>& tags = {});

    char AddImplicit(BoolExprBitVector<ArrN>& a);
};
