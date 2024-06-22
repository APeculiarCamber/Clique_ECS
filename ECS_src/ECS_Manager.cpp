// WARNING: THIS IS NOT A TRANSLATION FILE DUE TO THE TEMPLATES, IT IS AN IMPLEMENTATION FILE, INCLUDED DIRECTLY INTO THE HEADER, ECS_Manager.h

#include "ECS_Manager.h"

namespace ECS {

    template<typename Res, typename... Cs>
    template<class SContains, class SNotContains, auto SystemFunction>
    Base_SystemFunc *Manager<Res, ComponentList<Cs...>>::AddSystem() {
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