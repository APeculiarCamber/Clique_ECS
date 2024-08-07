#ifndef COMP_MANAGER_H
#define COMP_MANAGER_H

#include "BooleanExprTree.h"

#include <typeindex>
#include <type_traits>

#include "ComponentArray.h"

#include "UTILS_ECS.h"

template <typename... Cs>
class CoreComponentManager;

/**
 * GContains component list for contained components of a group
 */
template<typename... Types>
struct GContains {};
template <typename GContains>
struct IsGHasComponentList {
    static constexpr bool isNotValid = false;
};
template <typename... ConList>
struct IsGHasComponentList<GContains<ConList...>> {
    static constexpr bool isNotValid = true;
};

/**
 * GNotContains component list for components explicitly NOT CONTAINED IN a group
 */
template<typename... Types>
struct GNotContains {};
template <typename GNotContains>
struct IsGHasNotComponent {
    static constexpr bool isNotValid = false;
};
template <typename... NotConList>
struct IsGHasNotComponent<GNotContains<NotConList...>> {
    static constexpr bool isNotValid = true;
};

// TODO: TOPORT, write this over by the WriteToVec stuff.
template<typename C, typename... T>
struct g_extract_types {};
template<typename... Cs, typename... Types>
struct g_extract_types<CoreComponentManager<Cs...>, GContains<Types...>> {
    static constexpr void WriteTypesToVec(CoreComponentManager<Cs...>* mngr, std::vector<size_t>& outVec){
        ((outVec.push_back(mngr->template getTypeIndex<Types>())), ...);
    }
};
template<typename... Cs, typename... Types>
struct g_extract_types<CoreComponentManager<Cs...>, GNotContains<Types...>> {
    static constexpr void WriteTypesToVec(CoreComponentManager<Cs...>* mngr, std::vector<size_t>& outVec)  {
        ((outVec.push_back(mngr->template getTypeIndex<Types>())), ...);
    }
};

struct EntityMeta {
    uint32_t Handle;
    uint32_t UniqueID;
};

template <size_t N>
struct Entity {
    std::array<size_t, N> _components;
    EntityMeta _metaData;
    size_t _next;
};

template <size_t N>
struct ComponentModCommand {
    EntityMeta _meta = {0, 0};
    std::array<size_t, N> _new;
    std::array<size_t, N> _add;
    std::unordered_map<size_t, size_t> _addCompInds; // NOTE : if some MAX isNotValid, it is not an index which is available yet
};

#define DEFAULT_ENT_SIZE 256

template <size_t N>
class Base_ComponentArray;
template <typename T, size_t N, bool IsEmpty>
class ComponentArray;

/** Base case for recursion: empty list of types */
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
    /** Here it is, the ArrN you see in every file, this. is. it. */
    static constexpr size_t N = NUM_COMPS / (sizeof(size_t) * CHAR_BIT) + ((NUM_COMPS % (sizeof(size_t) * CHAR_BIT)) > 0 ? 1 : 0);

    // Function that returns the index of a provided type in the template parameter list TOPORT: rewrite this its giving
    template <typename T>
    static constexpr size_t getTypeIndex() {
        return getTypeIndexHelper<std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<std::remove_pointer_t<T>>>>,
                std::remove_const_t<std::remove_pointer_t<std::remove_cvref_t<std::remove_pointer_t<Cs>>>>...>(0);
    }

private:
    // Helper struct to recursively search for the index of a type in the template parameter list (AT COMPILE TIME!)
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
        assert(nullptr == "Type not found in template parameter list");
        return std::numeric_limits<size_t>::max();
    }

    array<unique_ptr<Base_ComponentArray<N>>, sizeof...(Cs)> _componentArrs{};

    size_t CURRENT_UNIQUE_ID = 1;

    std::vector<Entity<N>> m_entities;
    size_t m_entityFreeHead;

    std::vector<ComponentModCommand<N>> m_CompModCommands{};
    std::vector<size_t> m_modIndices;
    std::array<size_t, N> m_tags;

    BoolExprTreeManager<N> m_groupManager;
    bool m_groupsCommitted = false;

public:
    CoreComponentManager();

    [[maybe_unused]] const std::array<size_t, N>& GetTags() {  return m_tags; }
    [[maybe_unused]] const Entity<N>& GetEntity(size_t ind) { return m_entities[ind]; }
    template <typename C>
    ComponentArray<C, N>* GetComponentArray() {
        return static_cast<ComponentArray<C, N>*>(_componentArrs[getTypeIndex<C>()].get());
    }

    template <typename... As>
    void AddComponentBitVectorByTypes(std::array<size_t, N>* arr);

    template <typename... As>
    void RemoveComponentBitVectorByTypes(std::array<size_t, N>* arr);

    void UpdateFreeListForResize(size_t oldSize, size_t newSize);

    /*
    * GContains and GNotContains
    */
    template <typename HasComps, typename NotHasComps>
    char AddGroup(bool ifPartialAddimplicitly=false, bool ifPartialMakeTree=false);

    bool GroupsAreCommited() {
        return m_groupsCommitted;
    }

    bool CommitGroups();


    // Add an entity with the given components (NOTE: queued)
    template <typename... CompTypes>
    EntityMeta AddEntity(CompTypes*... cmps);

    // Delete an entity with handle and ID, MUST be valid combination of handle and ID (NOTE: queued)
    bool DeleteEntity(uint32_t handle, uint32_t uniqueID);

    // Add components of types CompTypes to the entity, using pointers to copied default values. (NOTE: queued)
    template <typename... CompTypes>
    bool AddComponents(EntityMeta meta, CompTypes*... cmps);

    // Delete components of CompType from entity (NOTE: queued)
    template <typename... CompTypes>
    bool DeleteComponents(EntityMeta meta);

    // Commit all queued changes to entities
    bool CommitEntityChanges();

    /*
    * If the entities signiture implies the system's, then the system can and should perform on it
    */
    bool EntityValidForSystem(size_t entHandle, BoolExprBitVector<N>& systemExpr) {
        return BitImplies(m_entities[entHandle]._components, systemExpr);
    }

    /*
    * Given a bool expr tree, add its nodes to the component arrays which are relevant to it!
    */
    void PropagateExprTreesToComponentArrays(BoolExprTree<N>* tree) {
        std::unordered_set<size_t> unhandledComponents(tree->GetExplicitlyAffectedComponents());

        BoolExprNode<N>* node = tree->GetRoot();
        while (node) {
            std::unordered_set<size_t> affectedComponents;
            GetHasBits(node->_bitRep, affectedComponents);

            // For each component which the current node affects explicitly,
            // if an above node hasn't already contibuted to a component, make this node the contributor!
            for (size_t comp : affectedComponents) {
                auto it = unhandledComponents.find(comp);
                if (it == unhandledComponents.end()) continue;
                unhandledComponents.erase(it);

                PRINT("Affixing for " << comp << " with tree " << tree << std::endl);
                _componentArrs[comp]->AffixExprTreeToComponentArray(node);
            }

            node = node->_left.get();
        }
    }
};

#include "CoreComponentManagerImpl.cpp"

#endif


// NOTES ON THE WORK:
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
	- UNLESS it's an ADD, then it gets put in place

We do removes first so nobody should be getting updated unduly or overwriting a new guy
	- THIS IS VITAL!!!!
	- I had already said it! but it was not well-formed... new version hopefully works

If new and old comp-type map to same group, Just replace in-place?
If they are different, then we don't need to worry??
We NEVER remove indexes from the sparse sets, just let them go stale?
	- but we do update them for ADDs and MOVEs
We could make periodic garbage collection, ad hoc.

If we really care about garbage for the sparse set, we can put all the TRULY deleted guys in a *special* array for removal AFTER EVERYTHING ELSE
*/
