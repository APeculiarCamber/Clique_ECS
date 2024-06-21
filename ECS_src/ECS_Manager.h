
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
        size_t _runLimit = 0, _currentRunCount = 0;

        Res* GetSharedResources() { return &_sharedResources; }

        template<class SContains, class SNotContains, auto SystemFunction>
        Base_SystemFunc* AddSystem()  {
            _componentManager.CommitGroups();

            using FunctionTraits = sys_function_traits<Res, N, ComponentList<Cs...>, decltype(SystemFunction)>;
            using ComponentArgs = typename FunctionTraits::arg_types;
            using FullArgs = typename FunctionTraits::full_arg_types;
            PRINT("HAS RESOURCES: " << FunctionTraits::UsesRes << std::endl);
            auto system = new SystemFunc<CoreComponentManager<Cs...>, Res, SContains, SNotContains, ComponentArgs, FullArgs,
                    FunctionTraits, SystemFunction>(&_componentManager, GetSharedResources());

            PRINT("CALLING UPDATE" << std::endl);
            system->UpdateAfterEntityCommits();
            PRINT("UPDATE ENDED" << std::endl);

            _systems.emplace_back(system);
            return _systems.back().get();
        }


        /*
        * MakeGroup attempts to create a group with a specified set of groups and WITHOUT another specified set of groups.
        *
        * MakeGroup is of the form: MakeGroup<GContains<...>, GNotContains<...>>();
        */
        template <typename HasComps, typename NotHasComps>
        __forceinline typename std::enable_if<IsGHasComponentList<HasComps>::isNotValid && IsGHasNotComponent<NotHasComps>::isNotValid, void>::type
        MakeGroup(bool ifPartialAddimplicitly = false, bool ifPartialMakeTree = false) {
            _componentManager.template AddGroup<HasComps, NotHasComps>(ifPartialAddimplicitly, ifPartialMakeTree);
        }

        /*
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

        /*
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

    template<typename Res, typename... Cs>
    template<typename... Ds>
    void Manager<Res, ComponentList<Cs...>>::DeleteComponents(EntityMeta meta) {
        _componentManager.template DeleteComponents<Ds...>(meta);
    }

    template<typename Res, typename... Cs>
    void Manager<Res, ComponentList<Cs...>>::SetTickLimit(size_t limit) {
        _runLimit = limit;
        if (limit == 0)
            _runLimit = std::numeric_limits<size_t>::max();
    }

    template<typename Res, typename... Cs>
    void Manager<Res, ComponentList<Cs...>>::CommitGroups() {
        _componentManager.CommitGroups();
    }

    template<typename Res, typename... Cs>
    void Manager<Res, ComponentList<Cs...>>::CommitEntityChanges() {
        // Update component lists
        assert(_componentManager.CommitEntityChanges());
        // Update system ptrs
        for (unique_ptr<Base_SystemFunc>& system : _systems) {
            system->UpdateAfterEntityCommits();
        }
    }


    template<typename Res, typename... Cs>
    void Manager<Res, ComponentList<Cs...>>::RunSystems() {
        _running = true;

        CommitGroups();
        CommitEntityChanges();

        for (size_t i = 0; i < _runLimit; ++i) {
            // Run systems
            for (unique_ptr<Base_SystemFunc>& system : _systems) {
                system->RunSystem();
            }
            // One shot systems for SHARED RESOURCES
            for (std::function<void(Res*, Manager<Res, ComponentList<Cs...>>*)>& f : _oneShotSystems) {
                f(&_sharedResources, this);
            }
            CommitEntityChanges();
            // DETERMINE IF WE WANT TO STOP
            _running &= _runLimit == 0 || (_currentRunCount++ <= _runLimit);
        }
        _running = false;
    }
};

#endif