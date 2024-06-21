//
// Created by idemaj on 6/21/24.
//

#include "ComponentArray.h"


template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr(BoolExprBitVector<N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (_hasBitRep) return BitImplies(bitRep, _bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
    return _parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->_bitRep);
}

template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr(array<size_t, N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (_hasBitRep) return BitImplies(bitRep, _bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
    return _parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->_bitRep);
}

template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr_AssumeParent(BoolExprBitVector<N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (_hasBitRep) return BitImplies(bitRep, _bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
    return BitImpliesNot(bitRep, matureSibling->_bitRep);
}

template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr_AssumeParent(array<size_t, N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (_hasBitRep) return BitImplies(bitRep, _bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (_parent->_left->_hasBitRep) ? _parent->_left : _parent->_right;
    return BitImpliesNot(bitRep, matureSibling->_bitRep);
}

template<typename C, size_t N>
template<bool HashedSet>
size_t ComponentGroupNode<C, N>::MakeRoom(SparseSet<C, HashedSet> &sset, int64_t leftRequest,
                                          ComponentGroupNode<C, N> **nextGroup) {

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

template<typename C, size_t N>
template<bool HashedSet>
void ComponentGroupNode<C, N>::CommitFillInUpdate(SparseSet<C, HashedSet> &sset, const vector<C> &registedComponents) {
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

template<typename C, size_t N>
ComponentGroupNode<C, N>::ComponentGroupNode(BoolExprBitVector<N> &rep, bool hasBitRep,
                                             BoolExprBitVector<N> &sharedGroups) : _bitRep(rep), _hasBitRep(hasBitRep) {
    _numSharedComponents = 0;

    for (size_t n = 0; n < N; ++n) {
        _componentsSharedWithGroup.at(n) = sharedGroups._CaresAbout.at(n) & sharedGroups._mustHave.at(n);

        std::bitset<sizeof(std::size_t) * CHAR_BIT> bits(_componentsSharedWithGroup.at(n));
        _numSharedComponents += bits.count();
    }
}




template<typename Comp, size_t N>
Comp *ComponentArray<Comp, N, false>::GetEntityComponent(size_t handle) {
    return _componentSet.Get(handle);
}

template<typename Comp, size_t N>
size_t ComponentArray<Comp, N, false>::RegisterNewComponent(Comp *comp) {
    _registeredNewComponents.push_back(*comp);
    return _registeredNewComponents.size() - 1;
}

template<typename Comp, size_t N>
bool
ComponentArray<Comp, N, false>::AddComponent(size_t entityHandle, size_t newCompInd, std::array<size_t, N> newCompID) {
    ComponentGroupNode<Comp, N>* newGroup = GetBestGroup(newCompID);
    newGroup->UPT_added.emplace_back(entityHandle, newCompInd);
    _compDiff++;
    return true;
}

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID) {
    ComponentGroupNode<Comp, N>* oldGroup = GetBestGroup(oldCompID);
    oldGroup->UPT_removed.push_back(entityHandle);
    _compDiff--;
    return true;
}

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID,
                                                   std::array<size_t, N> newCompID) {
    ComponentGroupNode<Comp, N>* oldBest = GetBestGroup(oldCompID);
    ComponentGroupNode<Comp, N>* newBest = GetBestGroup(newCompID);
    if (oldBest == newBest) return true;

    oldBest->UPT_removed.push_back(entityHandle);
    newBest->UPT_added.emplace_back(entityHandle, _registeredNewComponents.size());
    _registeredNewComponents.push_back(*_componentSet.Get(entityHandle));

    return true;
}

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd,
                                                                 std::array<size_t, N> oldCompID,
                                                                 std::array<size_t, N> newCompID) {

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

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::PercolateUpOffsets(ComponentGroupNode<Comp, N> *node) {
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

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::CommitComponentUpdates(size_t totalNumEntities) {
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

template<typename Comp, size_t N>
ComponentArray<Comp, N, false>::ComponentArray(size_t t) : componentIndex(t) {
    BoolExprBitVector<N> rootRep = MakeFromSpecVec({ componentIndex }, {});
    _root = new ComponentGroupNode<Comp, N>(rootRep, true, rootRep);
    CreateGroupLeavesArray();
}




template<typename Comp, size_t N>
Comp *ComponentArray<Comp, N, false>::GetComponentForEntity(size_t handle) {
    return _componentSet.Get(handle);
}

template<typename Comp, size_t N>
ComponentGroupNode<Comp, N> *ComponentArray<Comp, N, false>::GetBestGroup(BoolExprBitVector<N> &expr) {
    ComponentGroupNode<Comp, N>* best = _root;
    if (!best->ContainsExpr_AssumeParent(expr)) return nullptr;
    while (best->_left) {
        if (best->_left->ContainsExpr_AssumeParent(expr)) best = best->_left;
        else if (best->_right->ContainsExpr_AssumeParent(expr)) best = best->_right;
        else return best;
    }
    return best;
}

template<typename Comp, size_t N>
ComponentGroupNode<Comp, N> *ComponentArray<Comp, N, false>::GetBestGroup(array<size_t, N> &expr) {
    ComponentGroupNode<Comp, N>* best = _root;
    if (!best->ContainsExpr_AssumeParent(expr)) return nullptr;
    while (best->_left) {
        if (best->_left->ContainsExpr_AssumeParent(expr)) best = best->_left;
        else if (best->_right->ContainsExpr_AssumeParent(expr)) best = best->_right;
        else throw std::runtime_error("THIS SHOULD NOT HAPPEN: ENITIY ADDED AT NOT END");
    }
    return best;
}

template<typename Comp, size_t N>
ComponentGroupNode<Comp, N> *
ComponentArray<Comp, N, false>::AppendToNode(ComponentGroupNode<Comp, N> *myNode, BoolExprNode<N> *repNode,
                                             std::string s, bool isLeft) {
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

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::AppendLeavesInOrder(ComponentGroupNode<Comp, N> *node) {
    if (node->_left == nullptr) {
        _groupLeaves.push_back(node);
        PRINT(node->s << ":" << node->_startInd << "->" << node->_endInd << ", ");
        return;
    }
    AppendLeavesInOrder(node->_left);
    AppendLeavesInOrder(node->_right);
}

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::CreateGroupLeavesArray() {
    PRINT("Making Leave guy for " << componentIndex << ": ");
    _groupLeaves.clear();
    AppendLeavesInOrder(_root);
    _groupLeaves.push_back(nullptr); // closing \0
    PRINT(std::endl);
}

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::AffixExprTreeToComponentArray(BoolExprNode<N> *root) {
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

