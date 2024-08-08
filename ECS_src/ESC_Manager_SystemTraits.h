
#ifndef SYSTEM_TRAITS_H
#define SYSTEM_TRAITS_H

#include <type_traits>
#include <tuple>
#include <functional>
#include "CoreComponentManagerImpl.h"
#include "ECS_SystemImpl.h"

namespace ECS {
    template<typename... Cs>
    struct ComponentList {
    };

/**
 * These struct impls are used to statically analyze and determine properties of system functions.
 * It is aggressively SFINAE.
 *
 * GENERAL IDEA:
 * We have two ways to take input as a system function: Via tuple, or Via individual parameters; in both cases, we generate the required input function by meta-programming.
 * Systems can request the component manager; in that case, the system requires entity IDs.
 * Systems can request the single Resource, distinct from the component system; in that case we take special care to provide it.
 */

    template<typename Res, size_t N, typename CompList, typename Fn>
    struct sys_function_traits {
    };

/*********************************************************************************************
* FUNCTION Resource Types
**********************************************************************************************/


    template<typename R, size_t N, typename CompList, typename Res, typename... Args>
    struct sys_function_traits<Res, N, CompList, R(*)(std::tuple<Res *, Args...> &)> {
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
    template<typename R, size_t N, typename CompList, typename Res, typename... Args>
    struct sys_function_traits<Res, N, CompList, R(*)(Res *, Args...)> {
        using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
        using full_arg_types = FuncArgs<
                std::decay_t<std::remove_pointer_t<std::remove_cv_t<Res>>>,
                std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...
        >;
        static constexpr bool UsesRes = true;
        static constexpr bool UsesEntityID = false;
        static constexpr bool UsesTuple = false;
    };

    template<typename R, size_t N, typename... Cs, typename Res, typename... Args>
    struct sys_function_traits<Res, N, ComponentList<Cs...>, R(*)(
            std::tuple<Res *, size_t *, CoreComponentManager<Cs...> *, Args...> &)> {
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
    template<typename R, size_t N, typename... Cs, typename Res, typename... Args>
    struct sys_function_traits<Res, N, ComponentList<Cs...>, R(*)(Res *, size_t *, CoreComponentManager<Cs...> *,
                                                                  Args...)> {
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

    template<typename R, size_t N, typename CompList, typename Res, typename... Args>
    struct sys_function_traits<Res, N, CompList, R(*)(std::tuple<Args...> &)> {
        using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
        using full_arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
        static constexpr bool UsesRes = false;
        static constexpr bool UsesEntityID = false;
        static constexpr bool UsesTuple = true;
    };


    template<typename R, size_t N, typename CompList, typename Res, typename... Args>
    struct sys_function_traits<Res, N, CompList, R(*)(Args...)> {
        using arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
        using full_arg_types = FuncArgs<std::decay_t<std::remove_pointer_t<std::remove_cv_t<Args>>>...>;
        static constexpr bool UsesTuple = false;
        static constexpr bool UsesEntityID = false;
        static constexpr bool UsesRes = false;
    };
}

#endif