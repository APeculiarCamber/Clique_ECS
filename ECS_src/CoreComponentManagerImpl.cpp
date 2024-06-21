// WARNING: THIS IS NOT A TRANSLATION FILE DUE TO THE TEMPLATES, IT IS AN IMPLEMENTATION FILE, INCLUDED DIRECTLY INTO THE HEADER

#ifndef IMPL_COMP_MANAGER_CPP
#define IMPL_COMP_MANAGER_CPP
#include "CoreComponentManagerImpl.h"

template<typename... Cs>
CoreComponentManager<Cs...>::CoreComponentManager() : _entities(DEFAULT_ENT_SIZE), _modIndices(DEFAULT_ENT_SIZE)  {

    // CREATE THE COMPONENT ARRAYS
    ((_componentArrs[getTypeIndex<Cs>()].reset(static_cast<Base_ComponentArray<N>*>(new ComponentArray<Cs, N>(getTypeIndex<Cs>())))), ...);

    // Write the entity free list
    _entityFreeHead = 0;
    for (size_t i = 0; i < DEFAULT_ENT_SIZE; ++i) {
        _entities[i]._next = i + 1;
        _modIndices[i] = std::numeric_limits<size_t>::max();
    }

    // And the tags
    ((_tags.at(getTypeIndex<Cs>() / (sizeof(size_t) * CHAR_BIT))
              |= (std::is_empty_v<Cs> ? (size_t)1 : (size_t)0) << (getTypeIndex<Cs>() % (sizeof(size_t) * CHAR_BIT))), ...);

    // TODO: don't remember this
    _groupManager = BoolExprTreeManager<N>(_tags);
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
    if (_entities[meta.Handle]._metaData.UniqueID != meta.UniqueID) return false;
    PRINT("ATTEMPTING ADD FOR: " << meta.Handle << std::endl);

    // Get the mod index
    size_t modInd;
    if (_modIndices[meta.Handle] != std::numeric_limits<size_t>::max()) {
        PRINT("    ALREADY HAS A MOD RECORD!" << std::endl);
        modInd = _modIndices[meta.Handle];
    }
    else {
        modInd = _NEWAndADD_Components.size();
        _NEWAndADD_Components.emplace_back();
        _NEWAndADD_Components.back()._new = _entities[meta.Handle]._components;
    }
    _modIndices[meta.Handle] = modInd;

    // In the current scheme, if you try to add a component that it already has, that component will be overwritten, which is FINE?
    // Set the new bits for the component bit vector
    AddComponentBitVectorByTypes<As...>(&_NEWAndADD_Components[modInd]._new);
    AddComponentBitVectorByTypes<As...>(&_NEWAndADD_Components[modInd]._add);
    // Add all components to the component array
    (_NEWAndADD_Components[modInd]._addCompInds.insert_or_assign(getTypeIndex<As>(),
                                                                 static_cast<ComponentArray<As, N>*>(_componentArrs[getTypeIndex<As>()].get())->RegisterNewComponent(cmps)), ...);

    return true;
}

template<typename... Cs>
template<typename... As>
bool CoreComponentManager<Cs...>::DeleteComponents(EntityMeta meta) {
    if (_entities[meta.Handle]._metaData.UniqueID != meta.UniqueID) return false;

    PRINT("ATTEMPTING DELETE FOR: " << meta.Handle << std::endl);
    // Get the mod index
    size_t modInd;
    if (_modIndices[meta.Handle] != std::numeric_limits<size_t>::max()) {
        modInd = _modIndices[meta.Handle];
    }
    else {
        modInd = _NEWAndADD_Components.size();
        _NEWAndADD_Components.emplace_back();
        _NEWAndADD_Components.back()._new = _entities[meta.Handle]._components;
    }
    _modIndices[meta.Handle] = modInd;


    // remove from both, in a case of an add that we are now removing
    RemoveComponentBitVectorByTypes<As...>(&_NEWAndADD_Components[modInd]._new);
    RemoveComponentBitVectorByTypes<As...>(&_NEWAndADD_Components[modInd]._add);
    return true;
}

template<typename... Cs>
void CoreComponentManager<Cs...>::DEBUG_PRINT_RECORD(NEWandADDComponents<N> &mod) {
    PRINT("For Entity " << mod._meta.Handle << "(" << mod._meta.UniqueID << "): " << std::endl);
    unordered_set<size_t> has;
    GetHasBits(_entities[mod._meta.Handle]._components, has);
    PRINT("old: "); for (auto s : has) PRINT(s << ",");
    PRINT(" ::: ");

    has.clear();
    GetHasBits(mod._new, has);
    PRINT("new: ");  for (auto s : has) PRINT(s << ",");
    PRINT(" ::: ");

    has.clear();
    GetHasBits(mod._add, has);
    PRINT("add: ");  for (auto s : has) PRINT(s << ",");
    PRINT(std::endl);

    has.clear();
    std::array<size_t, N> toInform = mod._new | _entities[mod._meta.Handle]._components;
    GetHasBits(toInform, has);
    PRINT("TOINFORMS: ");  for (auto s : has) PRINT(s << ",");
    PRINT(std::endl);
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::CommitEntityChanges() {
    if (_NEWAndADD_Components.size() == 0) return false;
    PRINT("TRYING TO COMMIT ENTITIES" << std::endl);

    for (NEWandADDComponents<N>& mod : _NEWAndADD_Components) {
        // DEBUG_PRINT_RECORD(mod);

        // update the entity IDs
        Entity<N>* ent = &_entities[mod._meta.Handle];
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
        _modIndices[ent->_metaData.Handle] = std::numeric_limits<size_t>::max();
    }

    _NEWAndADD_Components.clear();

    bool good = true;
    for (unique_ptr<Base_ComponentArray<N>>& compArray : _componentArrs) {
        good &= compArray->CommitComponentUpdates(_entities.size());
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
    _groupsCommitted = false;
    return _groupManager.AddGroup(has, hasNot, ifPartialAddimplicitly, ifPartialMakeTree);
}




template<typename... Cs>
void CoreComponentManager<Cs...>::UpdateFreeListForResize(size_t oldSize, size_t newSize) {
    for (; oldSize < newSize; ++oldSize)
        _entities[oldSize]._next = oldSize + 1;
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::CommitGroups() {
    if (_groupsCommitted) return false;

    PRINT("*****COMMITTING TREES!" << std::endl);
    for (unique_ptr<BoolExprTree<N>>& tree : _groupManager._exprTrees) {
        SwingAtComponents(tree.get());
    }
    _groupsCommitted = true;
    return true;
}

template<typename... Cs>
template<typename... As>
EntityMeta CoreComponentManager<Cs...>::AddEntity(As *... cmps) {

    // If we are out of memory, resize the array
    if (_entities.size() <= _entityFreeHead) {
        size_t oldSize = _entities.size();
        _entities.resize(_entities.size() * (size_t)2);
        UpdateFreeListForResize(oldSize, _entities.size());
        _modIndices.resize(_entities.size(), std::numeric_limits<size_t>::max());
    }

    // Get entity from the free list
    size_t entityInd = _entityFreeHead;
    _entityFreeHead = _entities[entityInd]._next;
    size_t newUniqueID = CURRENT_UNIQUE_ID++;
    _entities[entityInd]._metaData.UniqueID = (uint32_t)newUniqueID; // update its unique ID before DELETE, shows that it will be KILLED!

    // Get the mod Ind for the current modification record, or make one and update the mod index tracker
    size_t modInd;
    if (_modIndices[entityInd] != std::numeric_limits<size_t>::max()) {
        modInd = _modIndices[entityInd];
    }
    else {
        modInd = _NEWAndADD_Components.size();
        _NEWAndADD_Components.emplace_back();
    }
    _modIndices[entityInd] = modInd;

    // Write to the mod record
    _NEWAndADD_Components[modInd]._meta.UniqueID = (uint32_t)newUniqueID;
    _NEWAndADD_Components[modInd]._meta.Handle = (uint32_t)entityInd;
    // Set the new bits for the component bit vector
    AddComponentBitVectorByTypes<As...>(&_NEWAndADD_Components[modInd]._new);
    _NEWAndADD_Components.back()._add = _NEWAndADD_Components[modInd]._new;
    // Add all components to the component array
    (_NEWAndADD_Components[modInd]._addCompInds.insert_or_assign(getTypeIndex<As>(),
                                                                 static_cast<ComponentArray<As, N>*>(_componentArrs[getTypeIndex<As>()].get())->RegisterNewComponent(cmps)), ...);

    return { (uint32_t)entityInd, (uint32_t)newUniqueID };
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::DeleteEntity(uint32_t handle, uint32_t uniqueID) {
    // If the entity ID doesnt match, dont bother
    if (_entities[handle]._metaData._uniqueID != uniqueID) return false;
    PRINT("Attempting to DELETE (" << handle << ", " << uniqueID << ") " << std::endl);
    // update free list head
    _entities[handle]._next = _entityFreeHead;
    _entityFreeHead = handle;
    // TONOTE: THIS MOST IMPORTANT LINE, prevents additional operations from being performed on the entity
    _entities[handle]._metaData._uniqueID = 0;
    // set an entity to be changed, apply as 0 components will get it deleted from everything
    _modIndices[handle] = _NEWAndADD_Components.size();
    _NEWAndADD_Components.emplace_back(); // assumes this sets the NEW and ADD to 0, which it should!
    _NEWAndADD_Components.back()._meta.Handle = handle;
    _NEWAndADD_Components.back()._meta._uniqueID = uniqueID;

    // All SHUOLD be 0s so that adds work correctly
    for (size_t i = 0; i < N; ++i) {
        assert(_NEWAndADD_Components.back()._new.at(i) == 0);
        assert(_NEWAndADD_Components.back()._add.at(i) == 0);
    }
}
#endif