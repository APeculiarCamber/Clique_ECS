#ifndef EXPR_TREE_H
#define EXPR_TREE_H


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
          _bitRep{ .mustHave = bitRep.mustHave, .caresAbout = bitRep.caresAbout }, _hasBitRep(true) {}

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
    *   NOTE : currently only adds to leaves, which is manageable but NOT ideal
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

    // Returns true if the boolean expression can be added somewhere with this as an ancestor
    bool CanAdd(BoolExprBitVector<ArrN>& bitRep);
    // Returns true if the boolean expression can be added explicitly (meaning directly and without modification)
    bool CanBeAddedAsExplicit(BoolExprBitVector<ArrN>& bitRep);
};

template <size_t ArrN>
class BoolExprTree {
protected:
    unordered_set<size_t> m_explicitAffectedComponents;
    unique_ptr<BoolExprNode<ArrN>> m_root = nullptr;

public:
    const unordered_set<size_t>& GetExplicitlyAffectedComponents() const { return m_explicitAffectedComponents; }
    BoolExprNode<ArrN>* GetRoot() const { return m_root.get(); }

    bool CouldBeAdded(BoolExprBitVector<ArrN>& a);

    bool hasAsExplicitComponent(size_t comp) {
        return m_explicitAffectedComponents.find(comp) != m_explicitAffectedComponents.end();
    }

    char Add(BoolExprBitVector<ArrN>& a, const std::array<size_t, ArrN>& tags = {});
    char AddImplicit(BoolExprBitVector<ArrN>& a);
};

// Funny trick to get template definitions in my "translation" files
#include "BooleanExprTree.cpp"

#endif