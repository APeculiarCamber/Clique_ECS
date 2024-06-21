#pragma once
#include "CoreComponentManagerImpl.h"
#include "SystemImpl.h"
#include <type_traits>
#include <tuple>
#include <functional>

namespace ECS {

	template <typename... Cs>
	struct ComponentList {};


	template <typename Res, size_t N, typename CompList, typename Fn>
	struct sys_function_traits {};

	//template <typename R, typename Res, typename... Args>
	//struct sys_function_traits<Res, R(*)(std::tuple<Res*, Args...>&)> : public sys_function_traits<Res, R(*)(Res*, Args...)> {
	//	static constexpr bool UsesTuple = true;
	//};


	/*********************************************************************************************
	* Resource Types
	**********************************************************************************************/
	template <typename R, size_t N, typename CompList, typename Res, typename... Args>
	struct sys_function_traits<Res, N, CompList, R(*)(std::tuple<Res*, Args...>&)>
	{
		using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		using full_arg_types = FuncArgs<
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Res>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...
		>;
		static constexpr bool UsesTuple = true;
		static constexpr bool UsesEntityID = false;
		static constexpr bool UsesRes = true;
	};



	// NONE TUPLE ARGS
	template <typename R, size_t N, typename CompList, typename Res, typename... Args>
	struct sys_function_traits<Res, N, CompList, R(*)(Res*, Args...)>
	{
		using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		using full_arg_types = FuncArgs<
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Res>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...
		>;
		static constexpr bool UsesRes = true;
		static constexpr bool UsesEntityID = false;
		static constexpr bool UsesTuple = false;
	};

	
	// TODO : EXPIREMENTAL CORE COMP ADDING UPS

	template <typename R, size_t N, typename... Cs, typename Res, typename... Args>
	struct sys_function_traits<Res, N, ComponentList<Cs...>, R(*)(std::tuple<Res*, size_t*, CoreComponentManager<Cs...>*, Args...>&)>
	{
		using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		using full_arg_types = FuncArgs<
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Res>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<size_t>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<CoreComponentManager<Cs...>>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...
		>;
		static constexpr bool UsesTuple = true;
		static constexpr bool UsesEntityID = true;
		static constexpr bool UsesRes = true;
	};


	
	// NONE TUPLE ARGS
	template <typename R, size_t N, typename... Cs, typename Res, typename... Args>
	struct sys_function_traits<Res, N, ComponentList<Cs...>, R(*)(Res*, size_t*, CoreComponentManager<Cs...>*, Args...)>
	{
		using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		using full_arg_types = FuncArgs<
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Res>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<size_t>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<CoreComponentManager<Cs...>>>>,
			std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...
		>;
		static constexpr bool UsesRes = true;
		static constexpr bool UsesTuple = false;
		static constexpr bool UsesEntityID = true;
	};

	/*********************************************************************************************
	* NON-Resource Types
	**********************************************************************************************/

	template <typename R, size_t N, typename CompList, typename Res, typename... Args>
	struct sys_function_traits<Res, N, CompList, R(*)(std::tuple <Args...>&)>
	{
		using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		using full_arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		static constexpr bool UsesRes = false;
		static constexpr bool UsesEntityID = false;
		static constexpr bool UsesTuple = true;
	};



	template <typename R, size_t N, typename CompList, typename Res, typename... Args>
	struct sys_function_traits<Res, N, CompList, R(*)(Args...)>
	{
		using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		using full_arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
		static constexpr bool UsesTuple = false;
		static constexpr bool UsesEntityID = false;
		static constexpr bool UsesRes = false;
	};


	/*********************************************************************************************************
	
	
	
	**********************************************************************************************************/


	template <auto Function>
	struct SysFuncPtr {
		using FuncType = typename decltype(Function);
	};
		

	template <class Res, typename CL>
	class Manager {};

	template <typename Res, typename... Cs>
	class Manager<Res, ComponentList<Cs...> > {
	public:
		static constexpr size_t N = CoreComponentManager<Cs...>::N;

		CoreComponentManager<Cs...> _componentManager;
		std::vector<Base_SystemFunc*> _systems;
		Res _sharedResources{};
		bool _running = false, _groupsCommited = false;
		size_t _runLimit = 0, _currentRunCount;

		~Manager() {
			for (Base_SystemFunc* system : _systems)
				delete system;
		}


		Res* GetSharedResources() {
			return &_sharedResources;
		}

		template<class SContains, class SNotContains, auto SystemFunction>
		Base_SystemFunc* AddSystem() {
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

			_systems.push_back(system);
			return _systems.back();
			
			return nullptr;
		}

		template <typename GContains>
		struct IsGHasComponentList {
			static constexpr bool isNotEmpty = false;
		};
		template <typename GNotContains>
		struct IsGHasNotComponent {
			static constexpr bool isNotEmpty = false;
		};
		template <typename... ConList>
		struct IsGHasComponentList<GContains<ConList...>> {
			static constexpr bool isNotEmpty = true;
		};
		template <typename... NotConList>
		struct IsGHasNotComponent<GNotContains<NotConList...>> {
			static constexpr bool isNotEmpty = true;
		};

		/*
		* MakeGroup attempts to create a group with a specified set of groups and WITHOUT another specified set of groups.
		* 
		* MakeGroup is of the form: MakeGroup<GContains<...>, GNotContains<...>>();
		*/
		template <typename HasComps, typename NotHasComps>
		__forceinline typename std::enable_if<IsGHasComponentList<HasComps>::value && IsGHasNotComponent<NotHasComps>::value, void>::type
		MakeGroup(bool ifPartialAddimplicitly = false, bool ifPartialMakeTree = false) {
			_componentManager.AddGroup<HasComps, NotHasComps>(ifPartialAddimplicitly, ifPartialMakeTree);
		}

		/*
		* MakeGroup attempts to create a group with a specified set of groups and WITHOUT another specified set of groups.
		*
		* MakeGroup is of the form: MakeGroup<GContains<...>, GNotContains<...>>();
		*/
		template <typename HasComps, typename NotHasComps>
		__forceinline typename std::enable_if<!IsGHasComponentList<HasComps>::value || !IsGHasNotComponent<NotHasComps>::value, void>::type
		MakeGroup(bool ifPartialAddimplicitly = false, bool ifPartialMakeTree = false) {
			throw new std::runtime_error("ERROR: MakeGroup called not in the form: MakeGroup<GContains<...>, GNotContains<...>>()");
		}

		std::vector<std::function<void(Res*, Manager<Res, ComponentList<Cs...>>*)>> _oneShotSystems;

		/*
		* Add a function with a specific signature: void void(Res*, Manager<Res, ComponentList<Cs...>>*).
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
		__forceinline EntityMeta AddEntity(As*... cmps) {
			return _componentManager.AddEntity(cmps...);
		}
		template <typename... As>
		__forceinline void AddComponents(EntityMeta meta, As*... cmps) {
			_componentManager.AddComponents(meta, cmps...);
		}
		__forceinline void DeleteEntity(EntityMeta meta) {
			_componentManager.DeleteEntity(meta.Handle, meta.UniqueID);
		}
		template <typename... Ds>
		__forceinline void DeleteComponents(EntityMeta meta) {
			_componentManager.DeleteComponents<Ds...>(meta);
		}

		CoreComponentManager<Cs...>* Components() {
			return &_componentManager;
		}

		void SetTickLimit(size_t limit) {
			_runLimit = limit;
			if (limit == 0) 
				_runLimit = std::numeric_limits<size_t>::max();
		}

		__forceinline void CommitGroups() {
			_componentManager.CommitGroups();
		}

		__forceinline void CommitEntityChanges() {
			// Update component lists
			bool updated = _componentManager.CommitEntityChanges();
			// Update system ptrs
			for (Base_SystemFunc* system : _systems) {
				system->UpdateAfterEntityCommits();
			}
		}

		void RunSystems() {
			_running = true;

			CommitGroups();
			CommitEntityChanges();

			for (size_t i = 0; i < _runLimit; ++i) {
				// Run systems
				for (Base_SystemFunc* system : _systems) {
					system->RunSystem();
				}
				// One shot systems for SHARED RESOURCES
				for (std::function<void(Res*, Manager<Res, ComponentList<Cs...>>*)>& f : _oneShotSystems) {
					f(&_sharedResources, this);
				}
				CommitEntityChanges();
				// DETERIME IF WE WANT TO STOP
				_running &= _runLimit == 0 || (_currentRunCount++ <= _runLimit);
			}
			_running = false;
		}
	};


};