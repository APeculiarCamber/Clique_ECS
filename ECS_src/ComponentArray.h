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
 * Sorting/Grouping class for a component of type C.
 * @tparam C
 * @tparam N
 */
template <typename C, size_t N>
struct ComponentGroupNode : Base_ComponentGroupNode<N> {
public:
    ComponentGroupNode(BoolExprBitVector<N>& rep, bool hasBitRep, BoolExprBitVector<N>& sharedGroups);

    bool IsEquivalent(BoolExprBitVector<N>& expr) override { return BitEquiv(_bitRep, expr); }

    size_t GetNumComponents() override { return _endInd - _startInd; }
    size_t GetNumShared() override {  return _numSharedComponents; }

    C* GetStart() { return _compStart + _startInd; }
    C* GetEnd() {  return _compStart + _endInd; }

    size_t* GetStartEntityIndPtr() { return _handlerStart + _startInd; }
    BoolExprBitVector<N>& GetBitExpr() override {  return _bitRep; }

    const std::array<size_t, N>& GetComponentsGroupShares() {  return _componentsSharedWithGroup;  }
    /*
    *  NOTE: NOT USED CURRENTLY!
    *  Return true if all the entities within this node group would be included
    */
    // bool IsContainedByExpr(BoolExprBitVector<N>& bitRep);
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

    /*
    * A key assumption this makes to work correctly is that _startInd is PUSHING just up against
    * the border of its left neighbor group.
    */
    template <bool HashedSet>
    size_t MakeRoom(SparseSet<C, HashedSet>& sset, int64_t leftRequest, ComponentGroupNode<C, N>** nextGroup);

    /*
    * ASSUMES MAKEROOM HAS ALREADY BEEN CALLED, BE CAREFUL!!!
    *
    */
    template <bool HashedSet>
    void CommitFillInUpdate(SparseSet<C, HashedSet>& sset, const std::vector<C>& registedComponents);

// TOPORT : see about scope
    BoolExprBitVector<N> _bitRep;
    std::array<size_t, N> _componentsSharedWithGroup;
    size_t _numSharedComponents;
    bool _hasBitRep;
    std::string s;

    ComponentGroupNode<C, N>* _parent = nullptr;
    unique_ptr<ComponentGroupNode<C, N>> _left = nullptr;
    unique_ptr<ComponentGroupNode<C, N>> _right = nullptr;

    C* _compStart = nullptr;
    size_t* _handlerStart = nullptr;

    size_t _startInd = 0, _endInd = 0;
    std::vector<std::pair<size_t, size_t>> UPT_added;
    std::vector<size_t> UPT_removed;

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
 * @tparam IsTag TRUE using SFINAE
 */
template <typename Comp, size_t N, bool IsTag = std::is_empty_v<Comp>>
class ComponentArray : public Base_ComponentArray<N>
{
protected:
    size_t componentIndex;
public:
    explicit ComponentArray(size_t t) : componentIndex(t) {}
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
    ComponentGroupNode<Comp, N>* AppendToNode(ComponentGroupNode<Comp, N>* myNode, BoolExprNode<N>* repNode, std::string s, bool isLeft) { return nullptr; }


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
public:
    constexpr static bool HashedSet = false;

    unique_ptr<ComponentGroupNode<Comp, N>> _root;
    std::vector<ComponentGroupNode<Comp, N>*> _groupLeaves;

    size_t componentIndex;
    std::string TypeName() override {
        return typeid(Comp).name();
    }
    size_t Index() override {
        return componentIndex;
    }

    explicit ComponentArray(size_t t);
    Comp* GetComponentForEntity(size_t handle);

    ComponentGroupNode<Comp, N>* GetBestGroup(BoolExprBitVector<N>& expr);
    ComponentGroupNode<Comp, N>* GetBestGroup(std::array<size_t, N>& expr);

    ComponentGroupNode<Comp, N>* AppendToNode(ComponentGroupNode<Comp, N>* myNode, BoolExprNode<N>* repNode, std::string s, bool isLeft);


    void AppendLeavesInOrder(ComponentGroupNode<Comp, N>* node);
    void CreateGroupLeavesArray();

    /*
    * Create the entire grouping tree for THIS component
    *
    * All children of a far left node should know that there shared group is the same as their parent!
    */
    void AffixExprTreeToComponentArray(BoolExprNode<N>* root);


    /*******************************************************************************************
    ENTITY COMPONENT MAANGEMENT
    *********************************************************************************************/

    SparseSet<Comp, HashedSet> _componentSet;
    int64_t _compDiff = 0;
    std::vector<Comp> _registeredNewComponents;

    Comp* GetEntityComponent(size_t handle);

    size_t RegisterNewComponent(Comp* comp);

    bool AddComponent(size_t entityHandle, size_t newCompInd, std::array<size_t, N> newCompID);
    bool RemoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID);
    bool MoveComponent(size_t entityHandle, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID);
    bool OverwriteComponentForHandle(size_t entityHandle, size_t newCompInd, std::array<size_t, N> oldCompID, std::array<size_t, N> newCompID);

    void PercolateUpOffsets(ComponentGroupNode<Comp, N>* node);

    bool CommitComponentUpdates(size_t totalNumEntities);
};

#include "ComponentArray.cpp"

#endif