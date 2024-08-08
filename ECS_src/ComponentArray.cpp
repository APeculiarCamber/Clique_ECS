// WARNING: DUE TO TEMPLATE PARAMETERS, THIS IS NOT A TRANSLATION FILE, IT IS AN IMPLEMENTATION INCLUDED DIRECTLY INTO THE HEADER, ComponentArray.h

#include "ComponentArray.h"


template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr(BoolExprBitVector<N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (m_hasBitRep) return BitImplies(bitRep, m_bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (m_parent->m_left->m_hasBitRep) ? m_parent->m_left : m_parent->m_right;
    return m_parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->m_bitRep);
}

template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr(array<size_t, N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (m_hasBitRep) return BitImplies(bitRep, m_bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (m_parent->m_left->m_hasBitRep) ? m_parent->m_left.get() : m_parent->m_right.get();
    return m_parent->ContainsExpr(bitRep) && BitImpliesNot(bitRep, matureSibling->m_bitRep);
}

template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr_AssumeParent(BoolExprBitVector<N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (m_hasBitRep) return BitImplies(bitRep, m_bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (m_parent->m_left->m_hasBitRep) ? m_parent->m_left.get() : m_parent->m_right.get();
    return BitImpliesNot(bitRep, matureSibling->m_bitRep);
}

template<typename C, size_t N>
bool ComponentGroupNode<C, N>::ContainsExpr_AssumeParent(array<size_t, N> &bitRep) {
    // Lower level expression IMPLY higher level containing expressions
    if (m_hasBitRep) return BitImplies(bitRep, m_bitRep);
    // If we are a bitrep-less child, we contain it iff: our parent contains it, and the explicit child contains NONE of it
    ComponentGroupNode<C, N>* matureSibling = (m_parent->m_left->m_hasBitRep) ? m_parent->m_left.get() : m_parent->m_right.get();
    return BitImpliesNot(bitRep, matureSibling->m_bitRep);
}

template<typename C, size_t N>
template<bool HashedSet>
size_t ComponentGroupNode<C, N>::MakeRoom(SparseSet<C, HashedSet> &sset, int64_t leftRequest,
                                          ComponentGroupNode<C, N> **nextGroup) {

    int64_t toReqFromRight = leftRequest + m_UPT_added.size() - m_UPT_removed.size();
    PRINT(leftRequest << "->" << m_str << "->" << toReqFromRight << ",");
    // If we need more room to the right, get it immediately
    if (toReqFromRight > 0 && *nextGroup != nullptr) {
        (*nextGroup)->MakeRoom(sset, toReqFromRight, std::next(nextGroup));
    }

    // Make unmatched removals, FROM SPECIFICALLY THE BACK, THIS INVARIANT IS IMPORTANT
    while (m_UPT_added.size() < m_UPT_removed.size()) {
        sset.DeleteSingle(sset.m_indexMap[m_UPT_removed.back()], --m_endInd);
        m_UPT_removed.pop_back();
    }

    // either shift in bulk to the left, or to the right (BUT NOT BEFORE GETTING OTHERS TO SHIFT MAYBE!!!)
    if (leftRequest < 0) {
        size_t startOfDest = m_startInd + leftRequest;
        size_t start = std::max(m_endInd + leftRequest, m_startInd);
        size_t end = m_endInd;
        // SHIFT LEFT
        sset.ShiftRegionTo(start, end, startOfDest);
        m_startInd += leftRequest, m_endInd += leftRequest;
    }
    else if (leftRequest > 0) {
        size_t startOfDest = m_endInd;
        size_t start = m_startInd;
        size_t end = std::min(m_startInd + leftRequest, m_endInd);
        // SHIFT RIGHT
        sset.ShiftRegionTo(start, end, startOfDest);
        m_startInd += leftRequest, m_endInd += leftRequest;
    }

    // only moveroom if it wasn't called already, the next callee will know that they have space to move into
    if (toReqFromRight <= 0 && *nextGroup != nullptr) {
        (*nextGroup)->MakeRoom(sset, toReqFromRight, std::next(nextGroup));
    }

    return m_startInd;
}

template<typename C, size_t N>
template<bool HashedSet>
void ComponentGroupNode<C, N>::CommitFillInUpdate(SparseSet<C, HashedSet> &sset, const vector<C> &registedComponents) {
    // Assumes MakeRoom already called, which would cause removed.size <= add.size
    while (m_UPT_removed.size() > 0) {
        PRINT("TRYING FOR SWAPPING REGISTERED COMPONENT: " << m_UPT_added.back().second << std::endl);
        sset.AddAtIndex(m_UPT_removed.back(), m_UPT_added.back().first, registedComponents[m_UPT_added.back().second]);
        m_UPT_added.pop_back();
        m_UPT_removed.pop_back();
    }

    // Only adds remain
    for (const auto& [handle, compID] : m_UPT_added) {
        sset.AddAtIndex(m_endInd++, handle, registedComponents[compID]);
    }

    m_UPT_added.clear();
    m_UPT_removed.clear();

    m_compStart = sset.m_components.data();
    m_handlerStart = sset.m_handlers.data();
}

template<typename C, size_t N>
ComponentGroupNode<C, N>::ComponentGroupNode(BoolExprBitVector<N> &rep, bool hasBitRep,
                                             BoolExprBitVector<N> &sharedGroups) : m_bitRep(rep), m_hasBitRep(hasBitRep) {
    m_numSharedComponents = 0;

    for (size_t n = 0; n < N; ++n) {
        m_componentsSharedWithGroup.at(n) = sharedGroups.caresAbout.at(n) & sharedGroups.mustHave.at(n);

        std::bitset<sizeof(std::size_t) * CHAR_BIT> bits(m_componentsSharedWithGroup.at(n));
        m_numSharedComponents += bits.count();
    }
}




template<typename Comp, size_t N>
Comp *ComponentArray<Comp, N, false>::GetEntityComponent(size_t handle) {
    return m_componentSet.Get(handle);
}

template<typename Comp, size_t N>
size_t ComponentArray<Comp, N, false>::RegisterNewComponent(Comp *comp) {
    m_registeredNewComponents.push_back(*comp);
    return m_registeredNewComponents.size() - 1;
}

template<typename Comp, size_t N>
bool
ComponentArray<Comp, N, false>::AddComponent(size_t entHandle, size_t newCompInd, std::array<size_t, N> newCompID) {
    ComponentGroupNode<Comp, N>* newGroup = GetBestGroup(newCompID);
    newGroup->AddUPTForAddition(entHandle, newCompInd);
    m_compDiff++;
    return true;
}

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID) {
    ComponentGroupNode<Comp, N>* oldGroup = GetBestGroup(oldCompID);
    oldGroup->AddUPTForDeletion(entityHandle);
    m_compDiff--;
    return true;
}

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID,
                                                   std::array<size_t, N> newCompID) {
    ComponentGroupNode<Comp, N>* oldBest = GetBestGroup(oldCompID);
    ComponentGroupNode<Comp, N>* newBest = GetBestGroup(newCompID);
    if (oldBest == newBest) return true;

    oldBest->AddUPTForDeletion(entityHandle);
    newBest->AddUPTForAddition(entityHandle, m_registeredNewComponents.size());
    m_registeredNewComponents.push_back(*m_componentSet.Get(entityHandle));

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
        *m_componentSet.Get(entityHandle) = m_registeredNewComponents[newCompInd];
        return true;
    }
    oldBest->AddUPTForDeletion(entityHandle);
    newBest->AddUPTForAddition(entityHandle, newCompInd);

    return true;
}

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::PercolateUpOffsets(ComponentGroupNode<Comp, N> *node) {
    if (!node->m_left) return;
    // Process bottom up
    PercolateUpOffsets(node->m_left.get());
    PercolateUpOffsets(node->m_right.get());

    assert(node->m_left->m_endInd == node->m_right->m_startInd);

    node->m_startInd = node->m_left->m_startInd;
    node->m_endInd = node->m_right->m_endInd;
    node->m_compStart = node->m_left->m_compStart;
    node->m_handlerStart = node->m_left->m_handlerStart;
}

template<typename Comp, size_t N>
bool ComponentArray<Comp, N, false>::CommitComponentUpdates(size_t totalNumEntities) {
    m_componentSet.SetEntitySize(totalNumEntities);
    PRINT("Component " << componentIndex << ": Adding " << m_compDiff << std::endl);

    // this makes a very good place to parallelize, since each group does not touch any other...
    m_componentSet.AccommodateAdd(m_compDiff);

    // Far left will propogate the MAKE room accordingly
    ComponentGroupNode<Comp, N>** farLeft = m_groupLeaves.data();
    PRINT("Room for " << componentIndex << " (leaves=" << m_groupLeaves.size() << "): ");
    m_groupLeaves.front()->MakeRoom(m_componentSet, 0, std::next(farLeft));
    PRINT(std::endl);
    PRINT("Room made, now commiting group changes: " << std::endl);
    for (size_t g = 0; g < (m_groupLeaves.size() - 1); g++) {
        m_groupLeaves[g]->CommitFillInUpdate(m_componentSet, m_registeredNewComponents);
    }
    PRINT("The final offsets for " << componentIndex << ": ");
    for (size_t g = 0; g < (m_groupLeaves.size() - 1); g++) {
        PRINT(m_groupLeaves[g]->m_startInd << "->" << m_groupLeaves[g]->m_str << "->" << m_groupLeaves[g]->m_endInd << ",");
    }
    PRINT(std::endl);
    for (size_t g = 0; g < (m_groupLeaves.size() - 1); g++) {
        Comp* start =  m_groupLeaves[g]->GetStart();
        Comp* end = m_groupLeaves[g]->GetEnd();
    }
    PRINT(std::endl);

    PercolateUpOffsets(m_root.get());

    m_registeredNewComponents.clear();
    m_compDiff = 0;
    return false;
}

template<typename Comp, size_t N>
ComponentArray<Comp, N, false>::ComponentArray(size_t t) : componentIndex(t) {
    BoolExprBitVector<N> rootRep = MakeFromSpecVec<N>(vector<size_t>{ componentIndex }, vector<size_t>{});
    m_root.reset(new ComponentGroupNode<Comp, N>(rootRep, true, rootRep));
    CreateGroupLeavesArray();
}




template<typename Comp, size_t N>
Comp *ComponentArray<Comp, N, false>::GetComponentForEntity(size_t handle) {
    return m_componentSet.Get(handle);
}

template<typename Comp, size_t N>
ComponentGroupNode<Comp, N> *ComponentArray<Comp, N, false>::GetBestGroup(BoolExprBitVector<N> &expr) {
    ComponentGroupNode<Comp, N>* best = m_root.get();
    if (!best->ContainsExpr_AssumeParent(expr)) return nullptr;
    while (best->m_left.get()) {
        if (best->m_left->ContainsExpr_AssumeParent(expr)) best = best->m_left.get();
        else if (best->m_right->ContainsExpr_AssumeParent(expr)) best = best->m_right.get();
        else return best;
    }
    return best;
}

template<typename Comp, size_t N>
ComponentGroupNode<Comp, N> *ComponentArray<Comp, N, false>::GetBestGroup(array<size_t, N> &expr) {
    ComponentGroupNode<Comp, N>* best = m_root.get();
    if (!best->ContainsExpr_AssumeParent(expr)) return nullptr;
    while (best->m_left.get()) {
        if (best->m_left->ContainsExpr_AssumeParent(expr)) best = best->m_left.get();
        else if (best->m_right->ContainsExpr_AssumeParent(expr)) best = best->m_right.get();
        else throw std::runtime_error("THIS SHOULD NOT HAPPEN: ENITIY ADDED AT NOT END");
    }
    return best;
}

template<typename Comp, size_t N>
ComponentGroupNode<Comp, N> *
ComponentArray<Comp, N, false>::AppendToNode(ComponentGroupNode<Comp, N> *myNode, BoolExprNode<N> *repNode,
                                             const std::string& s, bool isLeft) {
    if (repNode == nullptr) return nullptr;

    myNode->m_str = "~" + s + "~";
    PRINT("    Got for: " << myNode->m_hasBitRep << ": " << (myNode->m_bitRep.mustHave.at(0) & myNode->m_bitRep.caresAbout.at(0)) << ", " << s << " with " << myNode->GetNumShared() << " shares" << std::endl);
    if (repNode->_left) {
        // If isLeft, we should use the new bit rep for shared groups in components, but if not left, we should use our parents
        myNode->m_left.reset(new ComponentGroupNode<Comp, N>(
                repNode->_left->_bitRep, repNode->_left->_hasBitRep, isLeft ? repNode->_left->_bitRep : repNode->_bitRep));
        myNode->m_left->m_parent = myNode;

        AppendToNode(myNode->m_left.get(), repNode->_left.get(), s + "L", isLeft);
    }
    if (repNode->_right) {
        myNode->m_right.reset(new ComponentGroupNode<Comp, N>(
                repNode->_right->_bitRep, repNode->_right->_hasBitRep, repNode->_bitRep));
        myNode->m_right->m_parent = myNode;

        AppendToNode(myNode->m_right.get(), repNode->_right.get(), s + "R", false);
    }

    if (!repNode->_left && !repNode->_right) {
        assert(repNode->_parent == nullptr || (repNode->_parent->_left != nullptr && repNode->_parent->_right != nullptr));
        PRINT("    Leaf for: " << myNode->m_hasBitRep << ": " << (myNode->m_bitRep.mustHave.at(0) & myNode->m_bitRep.caresAbout.at(0)) << ", " << s << " with " << myNode->GetNumShared() << " shares" << std::endl);
    }
    return myNode;
}

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::AppendLeavesInOrder(ComponentGroupNode<Comp, N> *node) {
    if (node->m_left == nullptr) {
        m_groupLeaves.push_back(node);
        PRINT(node->m_str << ":" << node->m_startInd << "->" << node->m_endInd << ", ");
        return;
    }
    AppendLeavesInOrder(node->m_left.get());
    AppendLeavesInOrder(node->m_right.get());
}

template<typename Comp, size_t N>
void ComponentArray<Comp, N, false>::CreateGroupLeavesArray() {
    PRINT("Making Leave guy for " << componentIndex << ": ");
    m_groupLeaves.clear();
    AppendLeavesInOrder(m_root.get());
    m_groupLeaves.push_back(nullptr); // closing \0
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
        BoolExprNode<N>* left = root->_left.get();
        if (left) {
            m_root->m_left.reset(AppendToNode(new ComponentGroupNode<Comp, N>(left->_bitRep, left->_hasBitRep, left->_bitRep), root->_left.get(), std::to_string(componentIndex) + "*L", true));
            m_root->m_left->m_parent = m_root.get();
        }
        else if (root->_right.get()) {
            m_root->m_left.reset(new ComponentGroupNode<Comp, N>(m_root->m_bitRep, false, m_root->m_bitRep));
            m_root->m_left->m_str = std::to_string(componentIndex) + "*L";
            m_root->m_left->m_parent = m_root.get();
        }
        // **RIGHT**
        BoolExprNode<N>* right = root->_right.get();
        if (right) {
            // RIGHT takes the shared components of its root, NOT ITSELF
            m_root->m_right.reset(AppendToNode(new ComponentGroupNode<Comp, N>(right->_bitRep, right->_hasBitRep, root->_bitRep), root->_right.get(), std::to_string(componentIndex) + "*R", false));
            m_root->m_right->m_parent = m_root.get();
        }
        else if (root->_left.get()) {
            m_root->m_right.reset(new ComponentGroupNode<Comp, N>(m_root->m_bitRep, false, m_root->m_bitRep));
            m_root->m_right->m_str = std::to_string(componentIndex) + "*R";
            m_root->m_right->m_parent = m_root.get();
        }
    }




    else {
        PRINT("APPENDING TO EXISTING ROOT" << std::endl);
        // The root of the BoolExprTree is going to be the LEFT child of the single component root of this components array'm_str tree
        m_root->m_left.reset(AppendToNode(new ComponentGroupNode<Comp, N>(root->_bitRep, root->_hasBitRep, root->_bitRep), root, std::to_string(componentIndex) + "*L", true));
        m_root->m_right.reset(new ComponentGroupNode<Comp, N>(m_root->m_bitRep, false, m_root->m_bitRep));
        m_root->m_right->m_str = std::to_string(componentIndex) + "*R";
        m_root->m_right->m_parent = m_root.get();
    }
    CreateGroupLeavesArray();
}


