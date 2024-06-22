// WARNING: DUE TO TEMPLATE PARAMETERS, THIS IS NOT A TRANSLATION FILE, IT IS AN IMPLEMENTATION INCLUDED DIRECTLY INTO THE HEADER, BooleanExprTree.h


#include "BooleanExprTree.h"


template<size_t ArrN>
bool BoolExprNode<ArrN>::ContainsExpr(BoolExprBitVector<ArrN> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (_hasBitRep) return BitImplies(bitRep, _bitRep);
    // If we are a right child, we contain it iff: our parent contains it, and the left-explicit child contains NONE of it
    BoolExprNode<ArrN>* matureSibling = _parent->_left->_hasBitRep ? _parent->_left : _parent->_right;
    return _parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->_bitRep);
}

template<size_t ArrN>
bool BoolExprNode<ArrN>::ContainsExpr_AssumeParent(BoolExprBitVector<ArrN> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (_hasBitRep) return BitImplies(bitRep, _bitRep);
    // If we are a right child, we contain it iff: our parent contains it, and the left-explicit child contains NONE of it
    BoolExprNode<ArrN>* matureSibling = _parent->_left->_hasBitRep ? _parent->_left.get() : _parent->_right.get();
    return BitImpliesNot(bitRep, matureSibling->_bitRep);
}

template<size_t ArrN>
BoolExprNode<ArrN> *BoolExprNode<ArrN>::AddExpr(BoolExprBitVector<ArrN> &bitRep) {
    if (ContainsExpr_AssumeParent(bitRep)) {
        // Handle case of leaf
        if (_left == nullptr) {
            PRINT("    Adding to THIS node" << std::endl);
            _left.reset(new BoolExprNode<ArrN>(this, bitRep));
            _right.reset(new BoolExprNode<ArrN>(this));
            return _left.get();
        }
        else {
            if (_left->ContainsExpr_AssumeParent(bitRep)) {
                PRINT("    Going to LEFT node" << std::endl);
                return _left->AddExpr(bitRep);
            }
            else if (_right->ContainsExpr_AssumeParent(bitRep)) {
                PRINT("    Going to RIGHT node" << std::endl);
                return _right->AddExpr(bitRep);
            }
            else {
                assert(nullptr == "ERROR: We can only insert groups to the leaves currently. The added group cannot be.");
            }
        }


    }
    else {
        assert(nullptr == "ERROR: No ADD OCCURED.");
    }
    return nullptr;
}

template<size_t ArrN>
BoolExprNode<ArrN> *BoolExprNode<ArrN>::AddExprImplicit(BoolExprBitVector<ArrN> &bitRep, bool onFarLeft) {
    if (ContainsExpr_AssumeParent(bitRep)) {
        // Handle case of leaf
        if (_left == nullptr) {
            PRINT("    Adding IMPLICITLY to THIS node" << std::endl);
            if (onFarLeft) {
                _right.reset(new BoolExprNode<ArrN>(this, bitRep, false));
                _left.reset(new BoolExprNode<ArrN>(this));
                return _right.get();
            }
            else {
                _left.reset(new BoolExprNode<ArrN>(this, bitRep, false));
                _right.reset(new BoolExprNode<ArrN>(this));
                return _left.get();
            }
        }
        else {
            if (_left->ContainsExpr_AssumeParent(bitRep)) {
                PRINT("    Going to LEFT node" << std::endl);
                return _left->AddExprImplicit(bitRep, onFarLeft);
            }
            else if (_right->ContainsExpr_AssumeParent(bitRep)) {
                PRINT("    Going to RIGHT node" << std::endl);
                return _right->AddExprImplicit(bitRep, false);
            }
            else {
                assert(nullptr == "ERROR: We can only insert groups to the leaves currently. The added group cannot be.");
            }
        }


    }
    return nullptr;
}

template<size_t ArrN>
bool BoolExprNode<ArrN>::CanAdd(BoolExprBitVector<ArrN> &bitRep) {
    // Contained in a node and contains the next node

    if (ContainsExpr_AssumeParent(bitRep)) {
        if (_left == nullptr) return true;
        // if (_left->IsContainedByExpr(bitRep)) return true; // We can check on left, but NOT right
        //if (_right->IsContainedByExpr(bitRep)) return true;
        if (_left->ContainsExpr_AssumeParent(bitRep)) return _left->CanAdd(bitRep);
        if (_right->ContainsExpr_AssumeParent(bitRep)) return _right->CanAdd(bitRep);
        return false;
    }
    else return false;
}

template<size_t ArrN>
bool BoolExprNode<ArrN>::CanBeAddedAsExplicit(BoolExprBitVector<ArrN> &bitRep) {
    if (ContainsExpr_AssumeParent(bitRep)) {
        if (_left == nullptr) return _explicitRep;
        // if (_left->IsContainedByExpr(bitRep)) return _explicitRep;
        // if (_right->IsContainedByExpr(bitRep)) return false;
        if (_left->ContainsExpr_AssumeParent(bitRep)) return _left->CanBeAddedAsExplicit(bitRep);
        if (_right->ContainsExpr_AssumeParent(bitRep)) return false; // FALSE
        return false;
    }
    else return false;
}

template<size_t ArrN>
void BoolExprNode<ArrN>::PropogateExplicitComponents(unordered_set<size_t> &explicitComponents,
                                                     const array<size_t, ArrN> &tags) {
    if (!_explicitRep) return;
    if (_hasBitRep) {
        BoolExprBitVector<ArrN> explComps = _bitRep;
        for (size_t i = 0; i < ArrN; ++i) explComps._mustHave.at(i) &= ~tags.at(i);
        GetHasBits(explComps, explicitComponents);
        PRINT(_hasBitRep << std::endl);
    }
    if (_left) _left->PropogateExplicitComponents(explicitComponents);
}


/*****************************************************************
*********************************************************************/


template<size_t ArrN>
char BoolExprTree<ArrN>::Add(BoolExprBitVector<ArrN> &a, const array<size_t, ArrN> &tags) {
    BoolExprNode<ArrN>* newNode = nullptr;

    if (_root == nullptr) {
        PRINT("    Adding at root" << std::endl);
        _root.reset(new BoolExprNode<ArrN>(nullptr, a));
        newNode = _root.get();
    }
    else {
        PRINT("    Adding to tree" << std::endl);
        newNode = _root->AddExpr(a);
    }

    _explicitAffectedComponents.clear();
    _root->PropogateExplicitComponents(_explicitAffectedComponents, tags);
    PRINT("For this add, the tree has affected components: ");
    for (auto a : _explicitAffectedComponents) PRINT(a << ", ");
    PRINT(std::endl);

    if (newNode == nullptr) return FAILURE_ADD;
    else if (newNode->_explicitRep) return EXPLICIT_ADD;
    else return IMPLICIT_ADD;
}

template<size_t ArrN>
char BoolExprTree<ArrN>::AddImplicit(BoolExprBitVector<ArrN> &a) {
    BoolExprNode<ArrN>* newNode = nullptr;

    if (_root == nullptr) {
        assert(0 == "This is absolutely not OK: we cannot add implicitly add the root");
        PRINT("    Adding at root" << std::endl);
        _root.reset(new BoolExprNode<ArrN>(nullptr, a));
        newNode = _root.get();
    }
    else {
        PRINT("    Adding to tree IMPLICITLY" << std::endl);
        newNode = _root->AddExprImplicit(a);
    }

    // TODO : this should theoretically not be needed
    _explicitAffectedComponents.clear();
    _root->PropogateExplicitComponents(_explicitAffectedComponents);
    PRINT("For this add, the tree has affected components: ");
    for (auto a : _explicitAffectedComponents) PRINT(a << ", ");
    PRINT(std::endl);

    if (newNode == nullptr) return FAILURE_ADD;
    else return IMPLICIT_ADD;
}

template<size_t ArrN>
bool BoolExprTree<ArrN>::CouldBeAdded(BoolExprBitVector<ArrN> &a) {
    if (_root == nullptr) return true;
    // Check if this could become a new root
    // if (_root->IsContainedByExpr(a)) return true;
    // Check if it can be added
    return _root->CanAdd(a);
}