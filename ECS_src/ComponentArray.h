#pragma once
#include "BoolExprBitVector.h"
#include "BoolExprTreeManager.h"
#include "ComponentSparseSet.h"
#include <cassert>

template <size_t N>
struct Base_ComponentGroupNode {
	virtual bool IsEquivalent(BoolExprBitVector<N>& expr) = 0;
	virtual size_t GetNumComponents() = 0;
	virtual size_t GetNumShared() = 0;
	virtual size_t* GetStartEntityIndPtr() = 0;
	virtual BoolExprBitVector<N>& GetBitExpr() = 0;
};

template <typename C, size_t N>
struct ComponentGroupNode : Base_ComponentGroupNode<N> {
public:
	ComponentGroupNode(BoolExprBitVector<N>& rep, bool hasBitRep, BoolExprBitVector<N>& sharedGroups) : _bitRep(rep), _hasBitRep(hasBitRep) {
		_numSharedComponents = 0;

		for (size_t n = 0; n < N; ++n) {
			_componentsSharedWithGroup.at(n) = sharedGroups._CaresAbout.at(n) & sharedGroups._mustHave.at(n);

			std::bitset<sizeof(std::size_t) * CHAR_BIT> bits(_componentsSharedWithGroup.at(n));
			_numSharedComponents += bits.count();
		}
	}
	~ComponentGroupNode() {
		if (_left) delete _left;
		if (_right) delete _right;
	}

	constexpr bool IsEquivalent(BoolExprBitVector<N>& expr) override {
		return BitEquiv(_bitRep, expr);
	}
	size_t GetNumComponents() override {
		return _endInd - _startInd;
	}
	size_t GetNumShared() override {
		return _numSharedComponents;
	}

	C* GetStart() {
		// PRINT("Comp Get Start: " << _compStart + _startInd << std::endl);
		return _compStart + _startInd;
	}
	C* GetEnd() {
		return _compStart + _endInd;
	}

	size_t* GetStartEntityIndPtr() {
		return _handlerStart + _startInd;
	}

	BoolExprBitVector<N>& GetBitExpr() override {
		return _bitRep;
	}

	/**
	*  TODO: NOT USED CURRENTLY!
	*  Return true if all the entities within this node group would be included
	*/
	bool IsContainedByExpr(BoolExprBitVector<N>& bitRep) {
		// If this node expression implies the bit rep, it is contained by the bit rep.
		if (_hasBitRep) return BitImplies(_bitRep, bitRep);
		// ToDo ToDo ToDo ToDo
		return _parent->IsContainedByExpr(bitRep); // TODO TODO TODO: it is believed that we'll never need to check IsContained on a non-hasBitRep, but it DOESNT handle non-hasBitRep right now!
	}


	/**
	* Returns true if the node contains the bit representation ENTIRELY
	*
	* If System -> group
	*/
	bool ContainsExpr(BoolExprBitVector<N>& bitRep) {
		// Lower level expression IMPLY higher level containing expressions
		if (_hasBitRep) return BitImplies(bitRep, _bitRep);
		// If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
		ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
		return _parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->_bitRep);
	}

	/**
	* Returns true if the node contains the bit representation ENTIRELY
	*
	* If System -> group
	*/
	bool ContainsExpr(std::array<size_t, N>& bitRep) {
		// Lower level expression IMPLY higher level containing expressions
		if (_hasBitRep) return BitImplies(bitRep, _bitRep);
		// If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
		ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
		return _parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->_bitRep);
	}
	/**
	* Returns true if the node contains the bit representation ENTIRELY
	*
	* If System -> group
	*/
	bool ContainsExpr_AssumeParent(BoolExprBitVector<N>& bitRep) {
		// Lower level expression IMPLY higher level containing expressions
		if (_hasBitRep) return BitImplies(bitRep, _bitRep);
		// If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
		ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
		return true && BitImpliesNot(bitRep, matureSibling->_bitRep);
	}

	/**
	* Returns true if the node contains the bit representation ENTIRELY
	*
	* If System -> group
	*/
	bool ContainsExpr_AssumeParent(std::array<size_t, N>& bitRep) {
		// Lower level expression IMPLY higher level containing expressions
		if (_hasBitRep) return BitImplies(bitRep, _bitRep);
		// If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
		ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
		return true	&& BitImpliesNot(bitRep, matureSibling->_bitRep);
	}



	const std::array<size_t, N>& GetComponentsGroupShares() {
		return _componentsSharedWithGroup;
	}

	BoolExprBitVector<N> _bitRep;
	std::array<size_t, N> _componentsSharedWithGroup;
	size_t _numSharedComponents;
	bool _hasBitRep;
	std::string s;

	ComponentGroupNode<C, N>* _left = nullptr;
	ComponentGroupNode<C, N>* _parent = nullptr;
	ComponentGroupNode<C, N>* _right = nullptr;

	C* _compStart = nullptr;
	size_t* _handlerStart = nullptr;

	size_t _startInd = 0, _endInd = 0;
	std::vector<std::pair<size_t, size_t>> UPT_added;
	std::vector<size_t> UPT_removed;

	/*
	* A key assumption this makes to work correctly is that _startInd is PUSHING just up against
	* the border of its left neighbor group.
	*/
	template <bool HashedSet>
	size_t MakeRoom(SparseSet<C, HashedSet>& sset, int64_t leftRequest, ComponentGroupNode<C, N>** nextGroup) {

		int64_t toReqFromRight = leftRequest + UPT_added.size() - UPT_removed.size();
		PRINT(leftRequest << "->" << s << "->" << toReqFromRight << ",");
		// If we need more room to the right, get it immediately
		if (toReqFromRight > 0 && *nextGroup != nullptr) {
			(*nextGroup)->MakeRoom(sset, toReqFromRight, std::next(nextGroup));
		}

		// Make unmatched removals, FROM SPECIFICALLY THE BACK, THIS INVARIANT IS IMPORTANT
		while (UPT_added.size() < UPT_removed.size()) {
			sset.DeleteSingle(sset._indexMap[UPT_removed.back()], --_endInd);
			UPT_removed.pop_back();
		}

		// either shift in bulk to the left, or to the right (BUT NOT BEFORE GETTING OTHERS TO SHIFT MAYBE!!!)
		if (leftRequest < 0) {
			size_t startOfDest = _startInd + leftRequest;
			size_t start = std::max(_endInd + leftRequest, _startInd);
			size_t end = _endInd;
			// SHIFT LEFT
			sset.ShiftRegionTo(start, end, startOfDest);
			_startInd += leftRequest, _endInd += leftRequest;
		}
		else if (leftRequest > 0) {
			size_t startOfDest = _endInd;
			size_t start = _startInd;
			size_t end = std::min(_startInd + leftRequest, _endInd);
			// SHIFT RIGHT
			sset.ShiftRegionTo(start, end, startOfDest);
			_startInd += leftRequest, _endInd += leftRequest;
		}

		// only moveroom if it wasn't called already, the next callee will know that they have space to move into
		if (toReqFromRight <= 0 && *nextGroup != nullptr) {
			(*nextGroup)->MakeRoom(sset, toReqFromRight, std::next(nextGroup));
		}

		return _startInd;
	}

	/*
	* ASSUMES MAKEROOM HAS ALREADY BEEN CALLED, BE CAREFUL!!!
	* 
	*/ 
	template <bool HashedSet>
	void CommitFillInUpdate(SparseSet<C, HashedSet>& sset, const std::vector<C>& registedComponents) {
		// Assumes MakeRoom already called, which would cause removed.size <= add.size
		while (UPT_removed.size() > 0) {
			PRINT("TRYING FOR SWAPPING REGISTERED COMPONENT: " << UPT_added.back().second << std::endl);
			sset.AddAtIndex(UPT_removed.back(), UPT_added.back().first, registedComponents[UPT_added.back().second]);
			UPT_added.pop_back();
			UPT_removed.pop_back();
		}

		// Only adds remain
		for (const auto& [handle, compID] : UPT_added) {
			sset.AddAtIndex(_endInd++, handle, registedComponents[compID]);
		}

		UPT_added.clear();
		UPT_removed.clear();

		_compStart = sset._components.data();
		_handlerStart = sset._handlers.data();
	}
};




/*******************************************************************************************

********************************************************************************************/






template <size_t N>
class Base_ComponentArray {
public:
	virtual std::string TypeName() = 0;
	virtual size_t Index() = 0;

	virtual bool AddComponent(size_t entityHandle, size_t newCompInd, std::array<size_t, N> newCompID) = 0;
	virtual bool RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID) = 0;
	virtual bool MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) = 0;
	virtual bool OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) = 0;
	virtual bool CommitComponentUpdates(size_t tne) = 0;

	virtual void AffixExprTreeToComponentArray(BoolExprNode<N>* root) = 0;

	virtual ~Base_ComponentArray() {};
};


template <typename Comp, size_t N, bool IsTag = std::is_empty_v<Comp>>
class ComponentArray : public Base_ComponentArray<N>
{
	size_t componentIndex;
public:
	ComponentArray(size_t t) : componentIndex(t) {
		PRINT("CREATED EMPTY ARR: " << TypeName() << std::endl);
	}
	std::string TypeName() {
		return typeid(Comp).name();
	}
	size_t Index() {
		return componentIndex;
	}

	bool AddComponent(size_t entityHandle, size_t newCompInd, std::array<size_t, N> newCompID) {
		return true;
	}
	bool RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID) {
		return true;
	}
	bool MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) {
		return true;
	}
	bool OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) {
		return true;
	}
	bool CommitComponentUpdates(size_t tne) {
		return true;
	};

	void AffixExprTreeToComponentArray(BoolExprNode<N>* root) {};



	ComponentGroupNode<Comp, N>* GetBestGroup(BoolExprBitVector<N>& expr) {
		return nullptr;
	}

	ComponentGroupNode<Comp, N>* GetBestGroup(std::array<size_t, N>& expr) {
		return nullptr;
	}

	ComponentGroupNode<Comp, N>* AppendToNode(ComponentGroupNode<Comp, N>* myNode, BoolExprNode<N>* repNode, std::string s, bool isLeft) {
		return nullptr;
	}


	void AppendLeavesInOrder(ComponentGroupNode<Comp, N>* node) {
	}

	void CreateGroupLeavesArray() {
	}


	Comp* GetEntityComponent(size_t handle) {
		return nullptr;
	}

	size_t RegisterNewComponent(Comp* comp) {
		return -1;
	}
};
  

template <typename Comp, size_t N>
class  ComponentArray<Comp, N, false>: public Base_ComponentArray<N> {
public:
	constexpr static bool HashedSet = false;

	ComponentGroupNode<Comp, N>* _root;
	std::vector<ComponentGroupNode<Comp, N>*> _groupLeaves;

	

	size_t componentIndex;
	std::string TypeName() override {
		return typeid(Comp).name();
	}
	size_t Index() override {
		return componentIndex;
	}

	ComponentArray(size_t t) : componentIndex(t) {
		BoolExprBitVector<N> rootRep;
		MakeFromSpecVec({ componentIndex }, {}, rootRep);

		_root = new ComponentGroupNode<Comp, N>(rootRep, true, rootRep);

		CreateGroupLeavesArray();
	}

	~ComponentArray() override {
		PRINT("DELETING COMPONENT " << typeid(Comp).name() << std::endl);
		if (_root) delete _root;
	}


	Comp* GetComponentForEntity(size_t handle) {
		return _componentSet.Get(handle);
	}






	ComponentGroupNode<Comp, N>* GetBestGroup(BoolExprBitVector<N>& expr) {
		ComponentGroupNode<Comp, N>* best = _root;
		if (!best->ContainsExpr_AssumeParent(expr)) return nullptr;
		while (best->_left) {
			if (best->_left->ContainsExpr_AssumeParent(expr)) best = best->_left;
			else if (best->_right->ContainsExpr_AssumeParent(expr)) best = best->_right;
			else return best;
		}
		return best;
	}

	ComponentGroupNode<Comp, N>* GetBestGroup(std::array<size_t, N>& expr) {
		ComponentGroupNode<Comp, N>* best = _root;
		if (!best->ContainsExpr_AssumeParent(expr)) return nullptr;
		while (best->_left) {
			if (best->_left->ContainsExpr_AssumeParent(expr)) best = best->_left;
			else if (best->_right->ContainsExpr_AssumeParent(expr)) best = best->_right;
			else throw std::runtime_error("THIS SHOULD NOT HAPPEN: ENITIY ADDED AT NOT END");
		}
		return best;
	}





	// TODO : fix, then fix SContains and traversal for systems and entities
	ComponentGroupNode<Comp, N>* AppendToNode(ComponentGroupNode<Comp, N>* myNode, BoolExprNode<N>* repNode, std::string s, bool isLeft) {
		if (repNode == nullptr) return nullptr;

		myNode->s = "~" + s + "~";
		PRINT("    Got for: " << myNode->_hasBitRep << ": " << (myNode->_bitRep._mustHave.at(0) & myNode->_bitRep._CaresAbout.at(0)) << ", " << s << " with " << myNode->GetNumShared() << " shares" << std::endl);
		if (repNode->_left) {
			// If isLeft, we should use the new bit rep for shared groups in components, but if not left, we should use our parents
			myNode->_left = new ComponentGroupNode<Comp, N>(
				repNode->_left->_bitRep, repNode->_left->_hasBitRep, isLeft ? repNode->_left->_bitRep : repNode->_bitRep);
			myNode->_left->_parent = myNode;

			AppendToNode(myNode->_left, repNode->_left, s + "L", isLeft);
		}
		if (repNode->_right) {
			myNode->_right = new ComponentGroupNode<Comp, N>(
				repNode->_right->_bitRep, repNode->_right->_hasBitRep, repNode->_bitRep);
			myNode->_right->_parent = myNode;

			AppendToNode(myNode->_right, repNode->_right, s + "R", false);
		}

		if (!repNode->_left && !repNode->_right) {
			assert(repNode->_parent == nullptr || (repNode->_parent->_left != nullptr && repNode->_parent->_right != nullptr));
			PRINT("    Leaf for: " << myNode->_hasBitRep << ": " << (myNode->_bitRep._mustHave.at(0) & myNode->_bitRep._CaresAbout.at(0)) << ", " << s << " with " << myNode->GetNumShared() << " shares" << std::endl);
		}
		return myNode;
	}


	void AppendLeavesInOrder(ComponentGroupNode<Comp, N>* node) {
		if (node->_left == nullptr) {
			_groupLeaves.push_back(node);
			PRINT(node->s << ":" << node->_startInd << "->" << node->_endInd << ", ");
			return;
		}
		AppendLeavesInOrder(node->_left);
		AppendLeavesInOrder(node->_right);
	}

	void CreateGroupLeavesArray() {
		PRINT("Making Leave guy for " << componentIndex << ": ");
		_groupLeaves.clear();
		AppendLeavesInOrder(_root);
		_groupLeaves.push_back(nullptr); // closing \0
		PRINT(std::endl);
	}

	/*
	* Create the entire grouping tree for THIS component
	* 
	* All children of a far left node should know that there shared group is the same as their parent!
	*/
	void AffixExprTreeToComponentArray(BoolExprNode<N>* root) {
		// If they made a group of a single element, like a crazy person, get rid of that! 
		// (This could also happen when a partial/implicit group is requested).
		std::unordered_set<size_t> has, hasNot;
		GetHasAndHasNotBits(root->_bitRep, has, hasNot);
		assert(has.find(componentIndex) != has.end());
		if (has.size() == 1 && hasNot.size() == 0) {
			PRINT("USING MATCHED ROOT NODE" << std::endl);

			// **LEFT**
			BoolExprNode<N>* left = root->_left;
			if (left) {
				_root->_left = AppendToNode(new ComponentGroupNode<Comp, N>(left->_bitRep, left->_hasBitRep, left->_bitRep), root->_left, std::to_string(componentIndex) + "*L", true);
				_root->_left->_parent = _root;
			}
			else if (root->_right) {
				_root->_left = new ComponentGroupNode<Comp, N>(_root->_bitRep, false, _root->_bitRep);
				_root->_left->s = std::to_string(componentIndex) + "*L";
				_root->_left->_parent = _root;
			}
			// **RIGHT**
			BoolExprNode<N>* right = root->_right;
			if (right) {
				// RIGHT takes the shared components of its root, NOT ITSELF
				_root->_right = AppendToNode(new ComponentGroupNode<Comp, N>(right->_bitRep, right->_hasBitRep, root->_bitRep), root->_right, std::to_string(componentIndex) + "*R", false);
				_root->_right->_parent = _root;
			}
			else if (root->_left) {
				_root->_right = new ComponentGroupNode<Comp, N>(_root->_bitRep, false, _root->_bitRep);
				_root->_right->s = std::to_string(componentIndex) + "*R";
				_root->_right->_parent = _root;
			}
		}




		else {
			PRINT("APPENDING TO EXISTING ROOT" << std::endl);
			// The root of the BoolExprTree is going to be the LEFT child of the single component root of this components array's tree
			_root->_left = AppendToNode(new ComponentGroupNode<Comp, N>(root->_bitRep, root->_hasBitRep, root->_bitRep), root, std::to_string(componentIndex) + "*L", true);
			_root->_right = new ComponentGroupNode<Comp, N>(_root->_bitRep, false, _root->_bitRep);
			_root->_right->s = std::to_string(componentIndex) + "*R";
			_root->_right->_parent = _root;
		}
		CreateGroupLeavesArray();
	}








	/*******************************************************************************************
	ENTITY COMPONENT MAANGEMENT
	*********************************************************************************************/

	SparseSet<Comp, HashedSet> _componentSet;
	int64_t _compDiff = 0;
	std::vector<Comp> _registeredNewComponents;

	Comp* GetEntityComponent(size_t handle) {
		return _componentSet.Get(handle);
	}

	size_t RegisterNewComponent(Comp* comp) {
		_registeredNewComponents.push_back(*comp);
		return _registeredNewComponents.size() - 1;
	}


	// TODO : improve best group finding, could use hash map, or could improve locality and efficiency of checks by storing the nodes smartly?

	bool AddComponent(size_t entityHandle, size_t newCompInd, std::array<size_t, N> newCompID) {
		ComponentGroupNode<Comp, N>* newGroup = GetBestGroup(newCompID);
		newGroup->UPT_added.emplace_back(entityHandle, newCompInd);
		_compDiff++;
		return true;
	}

	bool RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID) {
		ComponentGroupNode<Comp, N>* oldGroup = GetBestGroup(oldCompID);
		oldGroup->UPT_removed.push_back(entityHandle);
		_compDiff--;
		return true;
	}

	bool MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) {
		ComponentGroupNode<Comp, N>* oldBest = GetBestGroup(oldCompID);
		ComponentGroupNode<Comp, N>* newBest = GetBestGroup(newCompID);
		if (oldBest == newBest) return true;

		oldBest->UPT_removed.push_back(entityHandle);
		newBest->UPT_added.emplace_back(entityHandle, _registeredNewComponents.size());
		_registeredNewComponents.push_back(*_componentSet.Get(entityHandle));

		return true;
	}
	bool OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) {

		ComponentGroupNode<Comp, N>* oldBest = GetBestGroup(oldCompID);
		ComponentGroupNode<Comp, N>* newBest = GetBestGroup(newCompID);
		if (oldBest == newBest) {
			// If we can, just do it simply in place,
			*_componentSet.Get(entityHandle) = _registeredNewComponents[newCompInd];
			return true;
		}
		oldBest->UPT_removed.push_back(entityHandle);
		newBest->UPT_added.emplace_back(entityHandle, newCompInd);

		return true;
	}

	void PercolateUpOffsets(ComponentGroupNode<Comp, N>* node) {
		if (!node->_left) return;
		// Process bottom up
		PercolateUpOffsets(node->_left);
		PercolateUpOffsets(node->_right);

		assert(node->_left->_endInd == node->_right->_startInd);

		node->_startInd = node->_left->_startInd;
		node->_endInd = node->_right->_endInd;
		node->_compStart = node->_left->_compStart;
		node->_handlerStart = node->_left->_handlerStart;
	}

	/*
	*/
	bool CommitComponentUpdates(size_t totalNumEntities) {
		_componentSet.SetEntitySize(totalNumEntities);
		PRINT("Component " << componentIndex << ": Adding " << _compDiff << std::endl);

		// this makes a very good place to parallelize, since each group does not touch any other...
		_componentSet.AccomodateAdd(_compDiff);

		// Far left will propogate the MAKE room accordingly
		ComponentGroupNode<Comp, N>** farLeft = _groupLeaves.data();
		PRINT("Room for " << componentIndex << " (leaves=" << _groupLeaves.size() << "): ");
		_groupLeaves.front()->MakeRoom(_componentSet, 0, std::next(farLeft));
		PRINT(std::endl);
		PRINT("Room made, now commiting group changes: " << std::endl);
		for (size_t g = 0; g < (_groupLeaves.size() - 1); g++) {
			_groupLeaves[g]->CommitFillInUpdate(_componentSet, _registeredNewComponents);
		}
		PRINT("The final offsets for " << componentIndex << ": ");
		for (size_t g = 0; g < (_groupLeaves.size() - 1); g++) {
			PRINT(_groupLeaves[g]->_startInd << "->" << _groupLeaves[g]->s << "->" << _groupLeaves[g]->_endInd << ",");
		}
		PRINT(std::endl);
		for (size_t g = 0; g < (_groupLeaves.size() - 1); g++) {
			Comp* start =  _groupLeaves[g]->GetStart();
			Comp* end = _groupLeaves[g]->GetEnd();
		}
		PRINT(std::endl);

		PercolateUpOffsets(_root);

		_registeredNewComponents.clear();
		_compDiff = 0;
		return false;
	}
};