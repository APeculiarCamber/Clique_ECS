#ifndef COMP_ARRAY_H
#define COMP_ARRAY_H

#include "BoolExprBitVector.h"
#include "BoolExprTreeManager.h"
#include "ComponentSparseSet.h" // TOPORT
#include <cassert>

template <size_t N>
struct Base_ComponentGroupNode {
    virtual bool IsEquivalent(BoolExprBitVector<N>& expr) = 0;
    virtual size_t GetNumComponents() = 0;
    virtual size_t GetNumShared() = 0;
    virtual size_t* GetStartEntityIndPtr() = 0;
    virtual BoolExprBitVector<N>& GetBitExpr() = 0;
};

/**
 * A group for the component.
 * Intentionally heavily coupled to the ComponentArray class.
 * Treated like a special struct.
 */
template <typename C, size_t N>
struct ComponentGroupNode : Base_ComponentGroupNode<N> {
public:
    ComponentGroupNode(BoolExprBitVector<N>& rep, bool hasBitRep, BoolExprBitVector<N>& sharedGroups);

    bool IsEquivalent(BoolExprBitVector<N>& expr) override { return BitEquiv(m_bitRep, expr); }

    size_t GetNumComponents() override { return m_endInd - m_startInd; }
    size_t GetNumShared() override {  return m_numSharedComponents; }

    C* GetStart() { return m_compStart + m_startInd; }
    C* GetEnd() {  return m_compStart + m_endInd; }

    size_t* GetStartEntityIndPtr() { return m_handlerStart + m_startInd; }
    BoolExprBitVector<N>& GetBitExpr() override {  return m_bitRep; }

    const std::array<size_t, N>& GetComponentsGroupShares() {  return m_componentsSharedWithGroup;  }

    /**
    * Returns true if the node contains the bit representation ENTIRELY
    *
    * If System -> group
    */
    bool ContainsExpr(BoolExprBitVector<N>& bitRep);
    /**
    * Returns true if the node contains the bit representation ENTIRELY
    *
    * If System -> group
    */
    bool ContainsExpr(std::array<size_t, N>& bitRep);
    /**
    * Returns true if the node contains the bit representation ENTIRELY
    *
    * If System -> group
    */
    bool ContainsExpr_AssumeParent(BoolExprBitVector<N>& bitRep);

    /**
    * Returns true if the node contains the bit representation ENTIRELY
    *
    * If System -> group
    */
    bool ContainsExpr_AssumeParent(std::array<size_t, N>& bitRep);

    /**
    * A key assumption this makes to work correctly is that m_startInd is PUSHING just up against
    * the border of its left neighbor group.
    */
    template <bool HashedSet>
    size_t MakeRoom(SparseSet<C, HashedSet>& sset, int64_t leftRequest, ComponentGroupNode<C, N>** nextGroup);

    /**
    * Commits the fill in. ASSUMES that MakeRoom was already called.
    */
    template <bool HashedSet>
    void CommitFillInUpdate(SparseSet<C, HashedSet>& sset, const std::vector<C>& registedComponents);

    /**
     * Add a component to pull from the ComponentArray's registered NEW list.
     * Poorly coupled but effective.
     *
     * @param entHandle Entity ID
     * @param newCompInd The INDEX of the component in the newly registered components.
     */
    void AddUPTForAddition(size_t entHandle, size_t newCompInd) { m_UPT_added.emplace_back(entHandle, newCompInd); }

    /** Add a component ID for removal. */
    void AddUPTForDeletion(size_t entHandle) { m_UPT_removed.push_back(entHandle); }

public:
    ComponentGroupNode<C, N>* m_parent = nullptr;
    unique_ptr<ComponentGroupNode<C, N>> m_left = nullptr;
    unique_ptr<ComponentGroupNode<C, N>> m_right = nullptr;
    BoolExprBitVector<N> m_bitRep;
    bool m_hasBitRep;
    std::string m_str;
    size_t m_startInd = 0, m_endInd = 0;
    C* m_compStart = nullptr;
    size_t* m_handlerStart = nullptr;

protected:
    std::array<size_t, N> m_componentsSharedWithGroup;
    size_t m_numSharedComponents;

    std::vector<std::pair<size_t, size_t>> m_UPT_added;
    std::vector<size_t> m_UPT_removed;

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

    virtual ~Base_ComponentArray() = default;
};

/**
 * A TAG storage class. Since tags don't have storage. Adding and removing elements is trivial and requires no re-organizing.
 * @tparam Comp An empty struct or class
 * @tparam N Number of total components in the ECS divided by the bit size of size_t
 * @tparam IsTag TRUE using SFINAE.
 *
 * See ComponentArray<Comp, N, false> below for the DATA component arrays.
 */
template <typename Comp, size_t N, bool IsTag = std::is_empty_v<Comp>>
class ComponentArray : public Base_ComponentArray<N>
{
protected:
    size_t componentIndex;
public:
    explicit ComponentArray(size_t t) : componentIndex(t) {}

    // WARNING: not guaranteed to be unique, use the ID system for that. This is for debugging.
    std::string TypeName() {   return typeid(Comp).name();  }
    size_t Index() {  return componentIndex;  }

    bool AddComponent(size_t entityHandle, size_t newCompInd, std::array<size_t, N> newCompID) { return true;  }
    bool RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID) {  return true; }
    bool MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) { return true; }
    bool OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID) { return true; }
    bool CommitComponentUpdates(size_t tne) {  return true;   };
    void AffixExprTreeToComponentArray(BoolExprNode<N>* root) {};

    ComponentGroupNode<Comp, N>* GetBestGroup(BoolExprBitVector<N>& expr) {  return nullptr;  }
    ComponentGroupNode<Comp, N>* GetBestGroup(std::array<size_t, N>& expr) {  return nullptr;  }
    ComponentGroupNode<Comp, N>* AppendToNode(ComponentGroupNode<Comp, N>* myNode, BoolExprNode<N>* repNode, const std::string& s, bool isLeft) { return nullptr; }


    void AppendLeavesInOrder(ComponentGroupNode<Comp, N>* node) { }
    void CreateGroupLeavesArray() { }


    Comp* GetEntityComponent(size_t handle) {  return nullptr; }
    size_t RegisterNewComponent(Comp* comp) { return -1; }
};

/**
 * A DATA storage class. Adding and removing components from this array is NOT TRIVIAL!
 * @tparam Comp The component type, filled with something
 * @tparam N The number of total components in the ECS divided by the bit size of size_t
 */
template <typename Comp, size_t N>
class  ComponentArray<Comp, N, false>: public Base_ComponentArray<N> {
protected:
    unique_ptr<ComponentGroupNode<Comp, N>> m_root;
    std::vector<ComponentGroupNode<Comp, N>*> m_groupLeaves;
    size_t componentIndex;

public:
    constexpr static bool HashedSet = false;

    explicit ComponentArray(size_t t);

    // WARNING: not guaranteed to be unique, use the ID system for that. This is for debugging.
    std::string TypeName() override {
        return typeid(Comp).name();
    }
    size_t Index() override {
        return componentIndex;
    }

    Comp* GetComponentForEntity(size_t handle);
    ComponentGroupNode<Comp, N>* GetBestGroup(BoolExprBitVector<N>& expr);
    ComponentGroupNode<Comp, N>* GetBestGroup(std::array<size_t, N>& expr);
    ComponentGroupNode<Comp, N>* AppendToNode(ComponentGroupNode<Comp, N>* myNode, BoolExprNode<N>* repNode, const std::string& s, bool isLeft);


    void AppendLeavesInOrder(ComponentGroupNode<Comp, N>* node);
    void CreateGroupLeavesArray();

    /**
    * Create the entire grouping tree for THIS component
    * All children of a far left node should know that there shared group is the same as their parent!
    */
    void AffixExprTreeToComponentArray(BoolExprNode<N>* root);


    /*******************************************************************************************
    ENTITY COMPONENT MANAGEMENT
    *********************************************************************************************/
protected:
    SparseSet<Comp, HashedSet> m_componentSet;
    int64_t m_compDiff = 0;
    std::vector<Comp> m_registeredNewComponents;
public:

    Comp* GetEntityComponent(size_t handle);

    /**
     *  Make copied temp storage for the component and return its ID.
     *  I have decided to make this the responsibility of the ComponentArray
     *    despite the coupling to the manager and the violation of SRP.
     */
    size_t RegisterNewComponent(Comp* comp);

    bool AddComponent(size_t entHandle, size_t newCompInd, std::array<size_t, N> newCompID);
    bool RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID);
    bool MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID);
    bool OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID);

    void PercolateUpOffsets(ComponentGroupNode<Comp, N>* node);

    bool CommitComponentUpdates(size_t totalNumEntities);
};

#include "ComponentArray.cpp"

#endif