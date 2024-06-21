#pragma once
#include "BooleanExprTree.h"

#include <typeindex>
#include <type_traits>

#include "ComponentArray.h"

#include "UTILS_ECS.h"

template <typename... Cs>
class CoreComponentManager;

template<typename... Types>
struct GContains {};

template<typename... Types>
struct GNotContains {
	// static void WriteTypesToVec(CoreComponentManager)
};

struct Gerr {

};

template<typename C, typename... T>
struct g_extract_types {};



struct EntityMeta { 
    uint32_t Handle;
    uint32_t UniqueID;
};

template <size_t N>
struct Entity {
	std::array<size_t, N> _components;
	EntityMeta _metaData;
    size_t _next; // TODO : we can union this later
};

template <size_t N>
struct NEWandADDComponents {
	EntityMeta _meta = {0, 0};
    std::array<size_t, N> _new;
    std::array<size_t, N> _add;
    std::unordered_map<size_t, size_t> _addCompInds; // TODO : if some MAX isNotEmpty, it is not an index which is available yet
};

#define DEFAULT_ENT_SIZE 256

template <size_t N>
class Base_ComponentArray;
template <typename T, size_t N, bool IsEmpty>
class ComponentArray;

// Base case for recursion: empty list of types
template <typename...>
struct all_classes
{
    static const bool value = true;
};
// Recursive case: check if the first type is a class, and continue with the rest of the list
template <typename T, typename... Ts>
struct all_classes<T, Ts...>
{
    static const bool value = std::is_class<T>() && all_classes<Ts...>::value;
};

template <typename... Cs>
class CoreComponentManager {
public:
    static_assert(all_classes<Cs...>::value, "All parameters must be structs.");
    static constexpr size_t NUM_COMPS = sizeof...(Cs);
    static constexpr size_t N = NUM_COMPS / (sizeof(size_t) * CHAR_BIT) + ((NUM_COMPS % (sizeof(size_t) * CHAR_BIT)) > 0 ? 1 : 0);

    // Function that returns the index of a provided type in the template parameter list
    template <typename T>
    static constexpr size_t getTypeIndex() {
        return getTypeIndexHelper<std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<std::remove_pointer_t<T>>>>,
            std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<std::remove_pointer_t<Cs>>>>...>(0);
    }

private:
    // Helper struct to recursively search for the index of a type in the template parameter list
    template <typename T, typename U, typename... Types>
    static constexpr size_t getTypeIndexHelper(size_t index) {
        if constexpr (std::is_same_v<T, U>) {
            return index;
        }
        else {
            return getTypeIndexHelper<T, Types...>(index + 1);
        }
    }

    template <typename T>
    static constexpr size_t getTypeIndexHelper(size_t index) {
        assert(0 == "Type not found in template parameter list");
        return std::numeric_limits<size_t>::max();
    }

    Base_ComponentArray<N>* _componentArrs[sizeof...(Cs)]{};

    size_t CURRENT_UNIQUE_ID = 1;

    std::vector<Entity<N>> _entities;
    size_t _entityFreeHead;
    std::vector<NEWandADDComponents<N>> _NEWAndADD_Components;
    std::vector<size_t> _modIndices;
	std::array<size_t, N> _tags;

	BoolExprTreeManager<N> _groupManager;

public:
    CoreComponentManager() /*: managerMap{{std::make_pair(std::type_index(typeid(Cs)), (Base_ComponentManager*)new ComponentManager<Cs>(ComponentBitMapping.find(std::type_index(typeid(Cs)))->second))...}}*/ {
        // Made map of component managers above!

        // CREATE THE COMPONENT ARRAYS
        ((_componentArrs[getTypeIndex<Cs>()] = new ComponentArray<Cs, N>(getTypeIndex<Cs>())), ...);
        for (int i = 0; i < sizeof...(Cs); ++i) {
            PRINT(_componentArrs[i]->Index() << ": " << _componentArrs[i]->TypeName() << std::endl);
        }

        _entities.resize(DEFAULT_ENT_SIZE);
        _modIndices.resize(DEFAULT_ENT_SIZE);

        _entityFreeHead = 0;
        for (size_t i = 0; i < DEFAULT_ENT_SIZE; ++i) {
            _entities[i]._next = i + 1;
            _modIndices[i] = std::numeric_limits<size_t>::max();
        }

		((_tags.at(getTypeIndex<Cs>() / (sizeof(size_t) * CHAR_BIT)) 
			|= (std::is_empty_v<Cs> ? (size_t)1 : (size_t)0) << (getTypeIndex<Cs>() % (sizeof(size_t) * CHAR_BIT))), ...);


		_groupManager = BoolExprTreeManager<N>(_tags);
        _NEWAndADD_Components.clear();
    }

	~CoreComponentManager() {
		PRINT("****************************" << std::endl << "*************************" << "DELL" << std::endl);
		for (Base_ComponentArray<N>* compArr : _componentArrs)
			delete compArr;
	}

	const std::array<size_t, N>& GetTags() {
		return _tags;
	}

	const Entity<N>& GetEntity(size_t ind) {
		return _entities[ind];
	}

	template <typename C> 
	ComponentArray<C, N>* GetComponentArray() {
		return static_cast<ComponentArray<C, N>*>(_componentArrs[getTypeIndex<C>()]);
	}

	template <typename... As>
	void AddComponentBitVectorByTypes(std::array<size_t, N>* arr) {
		((arr->at(getTypeIndex<As>() / (sizeof(size_t) * CHAR_BIT)) |= (size_t)1 << (getTypeIndex<As>() % (sizeof(size_t) * CHAR_BIT))), ...);
	}

	template <typename... As>
	void RemoveComponentBitVectorByTypes(std::array<size_t, N>* arr) {
		((arr->at(getTypeIndex<As>() / (sizeof(size_t) * CHAR_BIT)) &= ~((size_t)1 << (getTypeIndex<As>() % (sizeof(size_t) * CHAR_BIT)))), ...);
	}

	void UpdateFreeListForResize(size_t oldSize, size_t newSize) {
		for (; oldSize < newSize; ++oldSize)
			_entities[oldSize]._next = oldSize + 1;
	}

	bool _groupsCommitted = false;

	/*
	* GContains and GNotContains
	*/
	template <typename HasComps, typename NotHasComps>
	char AddGroup(bool ifPartialAddimplicitly=false, bool ifPartialMakeTree=false) {
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

	bool GroupsAreCommited() {
		return _groupsCommitted;
	}

	bool CommitGroups() {
		if (_groupsCommitted) return false;

		PRINT("*****COMMITTING TREES!" << std::endl);
		for (auto tree : _groupManager._exprTrees) {
			SwingAtComponents(tree);
		}
		_groupsCommitted = true;
		return true;
	}

	// TODO TODO TODO : MASSIVE TODO : fix this so that ADD doesn't affect us


	template <typename... As>
	EntityMeta AddEntity(As*... cmps) {

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
			static_cast<ComponentArray<As, N>*>(_componentArrs[getTypeIndex<As>()])->RegisterNewComponent(cmps)), ...);
		
		return { (uint32_t)entityInd, (uint32_t)newUniqueID };
	}

	bool DeleteEntity(uint32_t handle, uint32_t uniqueID) {
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

	template <typename... As>
	bool AddComponents(EntityMeta meta, As*... cmps);


	template <typename... As>
	bool DeleteComponents(EntityMeta meta);

	void DEBUG_PRINT_RECORD(NEWandADDComponents<N>& mod);

	bool CommitEntityChanges();






	/*
	* TODO:
	* If the entities signiture implies the system's, then the system can and should perform on it
	*/
	bool EntityValidForSystem(size_t entHandle, BoolExprBitVector<N>& systemExpr);

	/*
	* TODO : rename this
	* TODO : fix this to be better about right nodes and get those matched components stuff
	* Given a bool expr tree, add its nodes to the component arrays which are relevant to it!
	*/
	void SwingAtComponents(BoolExprTree<N>* tree);
};

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
bool CoreComponentManager<Cs...>::EntityValidForSystem(size_t entHandle, BoolExprBitVector<N> &systemExpr) {
    return BitImplies(_entities[entHandle]._components, systemExpr);
}

template<typename... Cs>
void CoreComponentManager<Cs...>::SwingAtComponents(BoolExprTree<N> *tree) {
    std::unordered_set<size_t> unhandledCompoents(tree->_explicitAffectedComponents);

    BoolExprNode<N>* node = tree->_root;
    while (node) {
        std::unordered_set<size_t> affectedComponents;
        GetHasBits(node->_bitRep, affectedComponents);

        // For each component which the current node affects explicitly,
        // if an above node hasn't already contibuted to a component, make this node the contributor!
        for (size_t comp : affectedComponents) {
            auto it = unhandledCompoents.find(comp);
            if (it == unhandledCompoents.end()) continue;
            unhandledCompoents.erase(it);

            PRINT("Affixxing for " << comp << " with tree " << tree << std::endl);
            _componentArrs[comp]->AffixExprTreeToComponentArray(node);
        }

        node = node->_left;
    }
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
bool CoreComponentManager<Cs...>::CommitEntityChanges() {
    if (_NEWAndADD_Components.size() == 0) return false;
    PRINT("TRYING TO COMMIT ENTITIES" << std::endl);

    for (NEWandADDComponents<N>& mod : _NEWAndADD_Components) {
        //DEBUG_PRINT_RECORD(mod);

        // update the entity IDs
        Entity<N>* ent = &_entities[mod._meta.Handle];
        ent->_metaData = mod._meta;

        std::array<size_t, N> toInform = mod._new | ent->_components; // TODO : fix this to a function if it dont compile

        for (size_t n = 0; n < N; ++n) {
            size_t info = toInform.at(n);
            size_t step = (n * (sizeof(size_t) * CHAR_BIT));

            while (info != 0) {
                unsigned long bit = 0;
                size_t oldN = ent->_components.at(n), newN = mod._new.at(n), addN = mod._add.at(n);

                _BitScanForward64(&bit, info);
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
    for (Base_ComponentArray<N>* compArray : _componentArrs) {
        good &= compArray->CommitComponentUpdates(_entities.size());
    }
    return true;
}

template<typename... Cs>
bool CoreComponentManager<Cs...>::CommitEntityChanges() {
    if (_NEWAndADD_Components.size() == 0) return false;
    PRINT("TRYING TO COMMIT ENTITIES" << std::endl);

    for (NEWandADDComponents<N>& mod : _NEWAndADD_Components) {
        //DEBUG_PRINT_RECORD(mod);

        // update the entity IDs
        Entity<N>* ent = &_entities[mod._meta.Handle];
        ent->_metaData = mod._meta;

        std::array<size_t, N> toInform = mod._new | ent->_components;

        for (size_t n = 0; n < N; ++n) {
            size_t info = toInform.at(n);
            size_t step = (n * (sizeof(size_t) * CHAR_BIT));

            while (info != 0) {
                size_t oldN = ent->_components.at(n), newN = mod._new.at(n), addN = mod._add.at(n);

                unsigned long bit = BitScanForward64(info);
                size_t bitMask = (((size_t)1) << bit);

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
    for (Base_ComponentArray<N>* compArray : _componentArrs) {
        good &= compArray->CommitComponentUpdates(_entities.size());
    }
    return true;
}

template<typename... Cs, typename... Types>
struct g_extract_types<CoreComponentManager<Cs...>, GContains<Types...>> {
	static constexpr void WriteTypesToVec(CoreComponentManager<Cs...>* mngr, std::vector<size_t>& outVec) {
		((outVec.push_back(mngr->getTypeIndex<Types>())), ...);
	}
};

template<typename... Cs, typename... Types>
struct g_extract_types<CoreComponentManager<Cs...>, GNotContains<Types...>> {
	static constexpr void WriteTypesToVec(CoreComponentManager<Cs...>* mngr, std::vector<size_t>& outVec) {
		((outVec.push_back(mngr->getTypeIndex<Types>())), ...);
	}
};

// TODO : INTEGRATE BEFORE LITERALLY ANYTHING ELSE YOU STUPID BITCH
// make | array
// for N
//    for bits
//       perform conditional add
// Commit to all components (can be done in parallel but the entity adds CANNOT!)
//		SAME FOR ENTITY ADDS UP DURING SYSTEM EXEC
// Update the entity component bits acoordingly
// Clear the mod array and any other things we need to
// 
// mod._addCompInds

/*
We will need to make an array of where to respond
	- this is:
		- all of the ADD component bits
		- all of the OLD component bits
		- all of the NEW component bits

For c in all-to-change:
	get the component manager, COMP
	if NEW has no c: COMP.DELETE(ent) 	   // add to delete array
	if OLD has no c: COMP.ADD(ent, ind)    // add to add array
	if ADD has no c: COMP.MOD(ent)	      // add to BOTH and write comp to back of ADD array, or DONT if same
	else 				: COMP.MOD_ADD(ent, ind)// add to BOTH (and we have ind already), or neither (AND WRITE DIRECT?)

If the group don't change for a component, it doesn't need to add it to its management arr
	- UNLESS its an ADD, then it gets put in place

We do removes first so nobody should be getting updated unduly or overwriting a new guy
	- THIS IS VITAL!!!!
	- holy shit I had already said it... but it was not well formed...

If new and old comp-type map to same group, Just replace in-place?
If they are different, then we don't need to worry??
We NEVER remove indexes from the sparse sets, just let them go stale?
	- but we do update them for ADDs and MOVEs
We could make periodic garbage collection, ad hoc.

If we really care about garbage for the sparse set, we can put all the TRULY deleted guys in a *special* array for removal AFTER EVERYTHING ELSE
*/