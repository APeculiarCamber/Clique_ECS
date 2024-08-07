// WARNING: THIS IS NOT A TRANSLATION FILE DUE TO THE TEMPLATES, IT IS AN IMPLEMENTATION FILE, INCLUDED DIRECTLY INTO THE HEADER, CoreComponentManagerImpl.h

#include "CoreComponentManagerImpl.h"

template<typename... Cs>
CoreComponentManager<Cs...>::CoreComponentManager() : m_entities(DEFAULT_ENT_SIZE), m_modIndices(DEFAULT_ENT_SIZE)  {

    // CREATE THE COMPONENT ARRAYS
    ((_componentArrs[getTypeIndex<Cs>()].reset(static_cast<Base_ComponentArray<N>*>(new ComponentArray<Cs, N>(getTypeIndex<Cs>())))), ...);

    // Write the entity free list
    m_entityFreeHead = 0;
    for (size_t i = 0; i < DEFAULT_ENT_SIZE; ++i) {
        m_entities[i]._next = i + 1;
        m_modIndices[i] = std::numeric_limits<size_t>::max();
    }

    // And the tags
    ((m_tags.at(getTypeIndex<Cs>() / (sizeof(size_t) * CHAR_BIT))
              |= (std::is_empty_v<Cs> ? (size_t)1 : (size_t)0) << (getTypeIndex<Cs>() % (sizeof(size_t) * CHAR_BIT))), ...);

    m_groupManager = BoolExprTreeManager<N>(m_tags);
}

template<typename... Cs>
template<typename... As>
void CoreComponentManager<Cs...>::RemoveComponentBitVectorByTypes(array<size_t, N> *arr) {
    ((arr->at(getTypeIndex<As>() / (sizeof(size_t) * CHAR_BIT)) |= (size_t)1 << (getTypeIndex<As>() % (sizeof(size_t) * CHAR_BIT))), ...);
}

template<typename... Cs>
template<typename... As>
void CoreComponentManager<Cs...>::AddComponentBitVectorByTypes(array<size_t, N> *arr) {
    ((arr->at(getTypeIndex<As>() / (sizeof(size_t) * CHAR_BIT)) &= ~((size_t)1 << (getTypeIndex<As>() % (sizeof(size_t) * CHAR_BIT)))), ...);
}

template<typename... Cs>
template<typename... As>
bool CoreComponentManager<Cs...>::AddComponents(EntityMeta meta, As *... cmps) {
    if (m_entities[meta.Handle]._metaData.UniqueID != meta.UniqueID) return false;
    PRINT("ATTEMPTING ADD FOR: " << meta.Handle << std::endl);

    // Get the mod index
    size_t modInd;
    if (m_modIndices[meta.Handle] != std::numeric_limits<size_t>::max()) {
        PRINT("    ALREADY HAS A MOD RECORD!" << std::endl);
        modInd = m_modIndices[meta.Handle];
    }
    else {
        modInd = m_CompModCommands.size();
        m_CompModCommands.emplace_back();
        m_CompModCommands.back()._new = m_entities[meta.Handle]._components;
    }
    m_modIndices[meta.Handle] = modInd;

    // In the current scheme, if you try to add a component that it already has, that component will be overwritten, which is FINE?
    // Set the new bits for the component bit vector
    AddComponentBitVectorByTypes<As...>(&m_CompModCommands[modInd]._new);
    AddComponentBitVectorByTypes<As...>(&m_CompModCommands[modInd]._add);
    // Add all components to the component array
    (m_CompModCommands[modInd]._addCompInds.insert_or_assign(getTypeIndex<As>(),
                                                             static_cast<ComponentArray<As, N>*>(_componentArrs[getTypeIndex<As>()].get())->RegisterNewComponent(cmps)), ...);

    return true;
}

template<typename... Cs>
template<typename... As>
bool CoreComponentManager<Cs...>::DeleteComponents(EntityMeta meta) {
    if (m_entities[meta.Handle]._metaData.UniqueID != meta.UniqueID) return false;

    PRINT("ATTEMPTING DELETE FOR: " << meta.Handle << std::endl);
    // Get the mod index
    size_t modInd;
    if (m_modIndices[meta.Handle] != std::numeric_limits<size_t>::max()) {
        modInd = m_modIndices[meta.Handle];
    }
    else {
        modInd = m_CompModCommands.size();
        m_CompModCommands.emplace_back();
        m_CompModCommands.back()._new = m_entities[meta.Handle]._components;
    }
    m_modIndices[meta.Handle] = modInd;


    // remove from both, in a case of an add that we are now removing
    RemoveComponentBitVectorByTypes<As...>(&m_CompModCommands[modInd]._new);
    RemoveComponentBitVectorByTypes<As...>(&m_CompModCommands[modInd]._add);
    return true;
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::CommitEntityChanges() {
    if (m_CompModCommands.size() == 0) return false;
    PRINT("TRYING TO COMMIT ENTITIES" << std::endl);

    for (ComponentModCommand<N>& mod : m_CompModCommands) {

        // update the entity IDs
        Entity<N>* ent = &m_entities[mod._meta.Handle];
        ent->_metaData = mod._meta;

        std::array<size_t, N> toInform = mod._new | ent->_components; // TODO : fix this to a function if it dont compile

        for (size_t n = 0; n < N; ++n) {
            size_t info = toInform.at(n);
            size_t step = (n * (sizeof(size_t) * CHAR_BIT));

            while (info != 0) {
                size_t oldN = ent->_components.at(n), newN = mod._new.at(n), addN = mod._add.at(n);

                unsigned long bit = BitScanForward64(info);
                size_t bitMask = (((size_t)1) << bit);
                // TODO:  ensure this works correctly for DELETES!!!!!
                bool isDel = (newN & bitMask) == 0;
                bool isAdd = (oldN & bitMask) == 0;
                bool isMove = (addN & bitMask) == 0;
                /*
                    if NEW has no c : COMP.DELETE(ent) 	   // add to delete array
                    if OLD has no c : COMP.ADD(ent, ind)    // add to add array
                    if ADD has no c : COMP.MOD(ent)	      // add to BOTH and write comp to back of ADD array, or DONT if same
                    else : COMP.MOD_ADD(ent, ind)// add to BOTH (and we have ind already), or neither (AND WRITE DIRECT?)
                */
                if (isDel)
                    _componentArrs[step + bit]->RemoveComponent(ent->_metaData.Handle, ent->_components);
                else if (isAdd) // todo : be care of this and overwrite, TODO: ensure that add comp ind are handled right IN HERE!!!
                    _componentArrs[step + bit]->AddComponent(ent->_metaData.Handle, mod._addCompInds[step + bit], mod._new);
                else if (isMove) {
                    PRINT("COMPONENT " << step + bit << " IS A MOVE!" << std::endl);
                    _componentArrs[step + bit]->MoveComponent(ent->_metaData.Handle, ent->_components, mod._new);
                }
                else
                    _componentArrs[step + bit]->OverwriteComponentForHandle(ent->_metaData.Handle, mod._addCompInds[step + bit], ent->_components, mod._new);
                // size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID

                // clear the processed bit
                info &= ~((size_t)1 << bit);
            }
        }
        // UPDATE THE COMPONENT ARRAY
        ent->_components = mod._new;

        // Reset the mod index
        m_modIndices[ent->_metaData.Handle] = std::numeric_limits<size_t>::max();
    }

    m_CompModCommands.clear();

    bool good = true;
    for (unique_ptr<Base_ComponentArray<N>>& compArray : _componentArrs) {
        good &= compArray->CommitComponentUpdates(m_entities.size());
    }
    return true;
}


template<typename... Cs>
template<typename HasComps, typename NotHasComps>
char CoreComponentManager<Cs...>::AddGroup(bool ifPartialAddimplicitly, bool ifPartialMakeTree) {
    std::vector<size_t> has, hasNot;
    g_extract_types<CoreComponentManager<Cs...>, HasComps>::WriteTypesToVec(this, has);
    g_extract_types<CoreComponentManager<Cs...>, NotHasComps>::WriteTypesToVec(this, hasNot);
    for (auto h : has)
        PRINT(", " << h);
    PRINT(std::endl);
    for (auto h : hasNot)
        PRINT(", " << h);
    PRINT(std::endl);
    m_groupsCommitted = false;
    return m_groupManager.AddGroup(has, hasNot, ifPartialAddimplicitly, ifPartialMakeTree);
}




template<typename... Cs>
void CoreComponentManager<Cs...>::UpdateFreeListForResize(size_t oldSize, size_t newSize) {
    for (; oldSize < newSize; ++oldSize)
        m_entities[oldSize]._next = oldSize + 1;
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::CommitGroups() {
    if (m_groupsCommitted) return false;

    PRINT("Commiting trees to components:" << std::endl);
    for (const unique_ptr<BoolExprTree<N>>& tree : m_groupManager.GetExpressionTrees()) {
        PropagateExprTreesToComponentArrays(tree.get());
    }
    m_groupsCommitted = true;
    return true;
}

template<typename... Cs>
template<typename... As>
EntityMeta CoreComponentManager<Cs...>::AddEntity(As *... cmps) {

    // If we are out of memory, resize the array
    if (m_entities.size() <= m_entityFreeHead) {
        size_t oldSize = m_entities.size();
        m_entities.resize(m_entities.size() * (size_t)2);
        UpdateFreeListForResize(oldSize, m_entities.size());
        m_modIndices.resize(m_entities.size(), std::numeric_limits<size_t>::max());
    }

    // Get entity from the free list
    size_t entityInd = m_entityFreeHead;
    m_entityFreeHead = m_entities[entityInd]._next;
    size_t newUniqueID = CURRENT_UNIQUE_ID++;
    m_entities[entityInd]._metaData.UniqueID = (uint32_t)newUniqueID; // update its unique ID before DELETE, shows that it will be KILLED!

    // Get the mod Ind for the current modification record, or make one and update the mod index tracker
    size_t modInd;
    if (m_modIndices[entityInd] != std::numeric_limits<size_t>::max()) {
        modInd = m_modIndices[entityInd];
    }
    else {
        modInd = m_CompModCommands.size();
        m_CompModCommands.emplace_back();
    }
    m_modIndices[entityInd] = modInd;

    // Write to the mod record
    m_CompModCommands[modInd]._meta.UniqueID = (uint32_t)newUniqueID;
    m_CompModCommands[modInd]._meta.Handle = (uint32_t)entityInd;
    // Set the new bits for the component bit vector
    AddComponentBitVectorByTypes<As...>(&m_CompModCommands[modInd]._new);
    m_CompModCommands.back()._add = m_CompModCommands[modInd]._new;
    // Add all components to the component array
    (m_CompModCommands[modInd]._addCompInds.insert_or_assign(getTypeIndex<As>(),
                                                             static_cast<ComponentArray<As, N>*>(_componentArrs[getTypeIndex<As>()].get())->RegisterNewComponent(cmps)), ...);

    return { (uint32_t)entityInd, (uint32_t)newUniqueID };
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::DeleteEntity(uint32_t handle, uint32_t uniqueID) {
    // If the entity ID doesnt match, dont bother
    if (m_entities[handle]._metaData._uniqueID != uniqueID) return false;
    PRINT("Attempting to DELETE (" << handle << ", " << uniqueID << ") " << std::endl);
    // update free list head
    m_entities[handle]._next = m_entityFreeHead;
    m_entityFreeHead = handle;
    // TONOTE: THIS MOST IMPORTANT LINE, prevents additional operations from being performed on the entity
    m_entities[handle]._metaData._uniqueID = 0;
    // set an entity to be changed, apply as 0 components will get it deleted from everything
    m_modIndices[handle] = m_CompModCommands.size();
    m_CompModCommands.emplace_back(); // assumes this sets the NEW and ADD to 0, which it should!
    m_CompModCommands.back()._meta.Handle = handle;
    m_CompModCommands.back()._meta._uniqueID = uniqueID;

    // All SHUOLD be 0s so that adds work correctly
    for (size_t i = 0; i < N; ++i) {
        assert(m_CompModCommands.back()._new.at(i) == 0);
        assert(m_CompModCommands.back()._add.at(i) == 0);
    }
}

