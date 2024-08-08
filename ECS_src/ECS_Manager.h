
#ifndef ECS_MANAGER_H
#define ECS_MANAGER_H

#include "ESC_Manager_SystemTraits.h"

namespace ECS {

    /**
     * Default manager class to system functions and the ECS.
     * @tparam Res The resource type, ALL global state.
     * @tparam CL Component list of components and tags
     */
    template <class Res, typename CL>
    class Manager {};

    template <typename Res, typename... Cs>
    class Manager<Res, ComponentList<Cs...> > {
    public:
        static constexpr size_t N = CoreComponentManager<Cs...>::N;

        CoreComponentManager<Cs...> _componentManager;
        std::vector<unique_ptr<Base_SystemFunc>> _systems;
        Res _sharedResources{};
        bool _running = false, _groupsCommited = false;
    protected:
        size_t _runLimit = 0, _currentRunCount = 0;
    public:
        size_t GetRunLimit() { return _runLimit; }
        size_t GetCurrentRunCount() { return _currentRunCount; }
        Res* GetSharedResources() { return &_sharedResources; }

        template<class SContains, class SNotContains, auto SystemFunction>
        Base_SystemFunc* AddSystem();


        /**
        * MakeGroup attempts to create a group with a specified set of groups and WITHOUT another specified set of groups.
        *
        * MakeGroup is of the form: MakeGroup<GContains<...>, GNotContains<...>>();
        */
        template <typename HasComps, typename NotHasComps>
        __forceinline typename std::enable_if<IsGHasComponentList<HasComps>::isNotValid && IsGHasNotComponent<NotHasComps>::isNotValid, void>::type
        MakeGroup(bool ifPartialAddimplicitly = false, bool ifPartialMakeTree = false) {
            _componentManager.template AddGroup<HasComps, NotHasComps>(ifPartialAddimplicitly, ifPartialMakeTree);
        }

        /**
        * MakeGroup attempts to create a group with a specified set of groups and WITHOUT another specified set of groups.
        *
        * MakeGroup is of the form: MakeGroup<GContains<...>, GNotContains<...>>();
        */
        template <typename HasComps, typename NotHasComps>
        __forceinline  typename std::enable_if<!IsGHasComponentList<HasComps>::isNotValid || !IsGHasNotComponent<NotHasComps>::isNotValid, void>::type
        MakeGroup(bool ifPartialAddimplicitly = false, bool ifPartialMakeTree = false) {
            throw new std::runtime_error("ERROR: MakeGroup called not in the form: MakeGroup<GContains<...>, GNotContains<...>>()");
        }

        std::vector<std::function<void(Res*, Manager<Res, ComponentList<Cs...>>*)>> _oneShotSystems;

        /**
        * Add a function with a specific signature: void(Res*, Manager<Res, ComponentList<Cs...>>*).
        * These systems are called at the end of each tick to perform cleanup, and global management.
        *
        * They DO NOT have access to entities or components, except to delete or add components to them.
        * Called before the final entity change commit of a tick.
        */
        bool AddCleanupSystem(std::function<void(Res*, Manager<Res, ComponentList<Cs...>>*)> func) {
            _oneShotSystems.push_back(func);
            return true;
        }

        template <typename... As>
        __forceinline  EntityMeta AddEntity(As*... cmps) {
            return _componentManager.AddEntity(cmps...);
        }
        template <typename... As>
        __forceinline  void AddComponents(EntityMeta meta, As*... cmps) {
            _componentManager.AddComponents(meta, cmps...);
        }
        __forceinline  void DeleteEntity(EntityMeta meta) {
            _componentManager.DeleteEntity(meta.Handle, meta.UniqueID);
        }
        template <typename... Ds>
        __forceinline  void DeleteComponents(EntityMeta meta);

        CoreComponentManager<Cs...>* Components() {
            return &_componentManager;
        }

        void SetTickLimit(size_t limit);

        __forceinline void CommitGroups();

        __forceinline void CommitEntityChanges();

        void RunSystems();
    };

};

#include "ECS_Manager.cpp"

#endif