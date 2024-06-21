#pragma once
#include <array>
#include <vector>
#include <unordered_set>
#include <bitset>
#include <cassert>
#include "PrintDebugger.h"

#define EXPLICIT_ADD 1
#define IMPLICIT_ADD 2
#define FAILURE_ADD 3

using namespace std;


template <size_t ArrN>
class BoolExprNode {
public:
	BoolExprNode<ArrN>* _parent = nullptr;
	BoolExprNode<ArrN>* _left = nullptr;
	BoolExprNode<ArrN>* _right = nullptr;
	bool _explicitRep = false, _hasBitRep = false;
	BoolExprBitVector<ArrN> _bitRep;

	BoolExprNode(BoolExprNode<ArrN>* parent) : _parent(parent), _bitRep() {
		_explicitRep = false, _hasBitRep = false; 
		_bitRep = {};
	}
	BoolExprNode(BoolExprNode<ArrN>* parent, BoolExprBitVector<ArrN>& bitRep, bool explRep=true) : _parent(parent) {
		_explicitRep = (parent == nullptr || (parent->_explicitRep) && explRep);

		_bitRep._CaresAbout = bitRep._CaresAbout;
		_bitRep._mustHave = bitRep._mustHave;
		_hasBitRep = true;
	}
	~BoolExprNode() {
		if (_left) delete _left;
		if (_right) delete _right;
	}

	/**
	* Collect all the component types which are included as ONs in the far left side EXPLICIT nodes
	*/
	void PropogateExplicitComponents(unordered_set<size_t>& explicitComponents, const std::array<size_t, ArrN>& tags = {}) {
		if (!_explicitRep) return;
		if (_hasBitRep) {
			BoolExprBitVector<ArrN> explComps = _bitRep;
			for (size_t i = 0; i < ArrN; ++i) explComps._mustHave.at(i) &= ~tags.at(i);
			GetHasBits(explComps, explicitComponents);
			PRINT(_hasBitRep << std::endl);
		}
		if (_left) _left->PropogateExplicitComponents(explicitComponents);
	}

	/**
	* Returns true if the node contains the bit representation
	*/
	bool ContainsExpr(BoolExprBitVector<ArrN>& bitRep) {
		// Lower level expression IMPLY higher level containing expressions
		if (_hasBitRep) return BitImplies(bitRep, _bitRep);
		// If we are a right child, we contain it iff: our parent contains it, and the left-explicit child contains NONE of it
		BoolExprNode<ArrN>* matureSibling = _parent->_left->_hasBitRep ? _parent->_left : _parent->_right;
		return _parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->_bitRep);
	}

	/**
	* Returns true if the node contains the bit representation
	*/
	bool ContainsExpr_AssumeParent(BoolExprBitVector<ArrN>& bitRep) {
		// Lower level expression IMPLY higher level containing expressions
		if (_hasBitRep) return BitImplies(bitRep, _bitRep);
		// If we are a right child, we contain it iff: our parent contains it, and the left-explicit child contains NONE of it
		BoolExprNode<ArrN>* matureSibling = _parent->_left->_hasBitRep ? _parent->_left : _parent->_right;
		return true && BitImpliesNot(bitRep, matureSibling->_bitRep);
	}

	/**
	*   TODO : currently only adds to leaves, which is manageable but NOT ideal
	*   Assumes TREE will handle a re-root, so it DOES NOT
	*	The main assumption this allows is parent has already done ContainedBy checks on its kids
	*/
	BoolExprNode<ArrN>* AddExpr(BoolExprBitVector<ArrN>& bitRep) {
		if (ContainsExpr_AssumeParent(bitRep)) {
			// Handle case of leaf
			if (_left == nullptr) {
				PRINT("    Adding to THIS node" << std::endl);
				_left = new BoolExprNode<ArrN>(this, bitRep);
				_right = new BoolExprNode<ArrN>(this);
				return _left;
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


	/**
	*   Assumes TREE will handle a re-root, so it DOES NOT
	*	The main assumption this allows is parent has already done ContainedBy checks on its kids
	*   
	*   This method inserts the bit rep, if it can and GUARENTEES that the 
	*/
	BoolExprNode<ArrN>* AddExprImplicit(BoolExprBitVector<ArrN>& bitRep, bool onFarLeft = true) {
		if (ContainsExpr_AssumeParent(bitRep)) {
			// Handle case of leaf
			if (_left == nullptr) {
				PRINT("    Adding IMPLICITLY to THIS node" << std::endl);
				if (onFarLeft) {
					_right = new BoolExprNode<ArrN>(this, bitRep, false);
					_left = new BoolExprNode<ArrN>(this);
					return _right;
				}
				else {
					_left = new BoolExprNode<ArrN>(this, bitRep, false);
					_right = new BoolExprNode<ArrN>(this);
					return _left;
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


	/**
	* TODO: these functions do not currently support IsContained by, since AddExpr also doesn't yet
	*/
	bool CanAdd(BoolExprBitVector<ArrN>& bitRep) {
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

	bool CanBeAddedAsExplicit(BoolExprBitVector<ArrN>& bitRep) {
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
};

template <size_t ArrN>
class BoolExprTree {
public:
	// list of explicit components
	// root
	unordered_set<size_t> _explicitAffectedComponents;
	BoolExprNode<ArrN>* _root = nullptr;

	~BoolExprTree() {
		PRINT("TRYING TO DELETE ROOT: " << _root << std::endl);
		if (_root) delete _root;
	}

	bool CouldBeAdded(BoolExprBitVector<ArrN>& a) {
		if (_root == nullptr) return true;
		// Check if this could become a new root
		// if (_root->IsContainedByExpr(a)) return true;
		// Check if it can be added
		return _root->CanAdd(a);
	}

	bool WillBeAddedAsExplicit(BoolExprBitVector<ArrN>& a) {
		if (_root == nullptr) return true;
		// Check if this could become a new root
		if (_root->IsContainedByExpr(a)) return true;
		// Check if it will be added as an explicit
		return _root->CanAddAsExplicit(a);
	}

	bool hasAsExplicitComponent(size_t comp) {
		return _explicitAffectedComponents.find(comp) != _explicitAffectedComponents.end();
	}

	char Add(BoolExprBitVector<ArrN>& a, const std::array<size_t, ArrN>& tags = {}) {
		BoolExprNode<ArrN>* newNode = nullptr;

		if (_root == nullptr) {
			PRINT("    Adding at root" << std::endl);
			_root = new BoolExprNode<ArrN>(nullptr, a);
			newNode = _root;
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

	char AddImplicit(BoolExprBitVector<ArrN>& a) {
		BoolExprNode<ArrN>* newNode = nullptr;

		if (_root == nullptr) {
			assert(0 == "This is absolutely not OK: we cannot add implicitly add the root");
			PRINT("    Adding at root" << std::endl);
			_root = new BoolExprNode<ArrN>(nullptr, a);
			newNode = _root;
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
};

