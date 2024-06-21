#pragma once

#include "ComponentArray.h"
#include "BoolExprBitVector.h"
#include "CoreComponentManagerImpl.h"
#include <tuple>
#include <functional>
#include <utility>

using namespace std;

// Meta-function to extract the argument types of a function and create a tuple of types
#include <typeindex>
template <typename Func>
struct FunctionTraits;

namespace meta {
    // Partial template specialization to extract the underlying type of Abs from a pointer to Abs
    template <typename T>
    struct underlying_type;

    template <typename T, size_t N>
    struct underlying_type<ComponentArray<T, N>*> {
        using type = T;
    };
};

template<typename... Types>
struct SContains {};

template<typename... Types>
struct SNotContains {};

template<typename... Types>
struct FuncArgs {};

template<typename T>
struct extract_types {};

template<typename... Types>
struct extract_types<SContains<Types...>> {
    using type = std::tuple<Types...>;
};

template<typename... Types>
struct extract_types<SNotContains<Types...>> {
    using type = std::tuple<Types...>;
};



template<typename... Ts, typename... Cs, std::size_t... Is>
auto conditional_tuple_assign_impl(const std::tuple<Ts...>& t1, const std::tuple<Ts...>& t2, const std::tuple<Cs...>& bools, std::index_sequence<Is...>) {
    static_assert(sizeof...(Ts) == sizeof...(Cs), "Number of elements in bool tuple must match number of elements in other tuples");

    return std::make_tuple((std::get<Is>(bools) ? std::get<Is>(t1) : std::get<Is>(t2))...);
}

template<typename... Ts, typename... Cs>
auto conditional_tuple_assign(const std::tuple<Ts...>& t1, const std::tuple<Ts...>& t2, const std::tuple<Cs...>& bools) {
    return conditional_tuple_assign_impl(t1, t2, bools, std::make_index_sequence<sizeof...(Ts)>());
}

template <std::size_t N>
struct BoolTupleGenerator {
    using type = decltype(std::tuple_cat(
            std::declval<std::tuple<bool>>(),
            std::declval<typename BoolTupleGenerator<N - 1>::type>()
    ));
};

// Define the base case of the recursive struct for when `N == 0`
template <>
struct BoolTupleGenerator<0> {
    using type = std::tuple<>;
};

class Base_SystemFunc {
public:
    virtual void RunSystem() = 0;
    virtual void RunFullGroups() = 0;
    virtual void RunPartialEquivGroups() = 0;
    virtual void RunBlindGroups() = 0;

    virtual bool HasFullEquivGroup() = 0;
    virtual bool HasPartialEquivGroup() = 0;
    bool HasOnlyBlindGroups() { return !HasPartialEquivGroup(); }
    void AssertFullEquiv() {  assert(HasFullEquivGroup()); }

    virtual void UpdateAfterEntityCommits() = 0;
};

void System_RunFullGroups(Base_SystemFunc* func) {
    func->RunFullGroups();
}
void System_RunPartialGroups(Base_SystemFunc* func) {
    func->RunPartialEquivGroups();
}
void System_RunBlindGroups(Base_SystemFunc* func) {
    func->RunBlindGroups();
}

template <typename Manager, class SharedResources, class SContains, class SNotContains, class CompFuncArgs, class FullFuncArgs, typename FuncTraits, auto func>
struct SystemFunc {
};

template<typename... AllCs, class SharedResources, class... ConCs, class... NotConCs, typename... Args, typename... FullArgs, typename FuncTraits, auto func>
class SystemFunc<CoreComponentManager<AllCs...>, SharedResources, SContains<ConCs...>, SNotContains<NotConCs...>, FuncArgs<Args...>, FuncArgs<FullArgs...>, FuncTraits, func> : public Base_SystemFunc {
public:
    static constexpr size_t N = CoreComponentManager<AllCs...>::N;
    using CoreCompManager = CoreComponentManager<AllCs...>;

    using func_type_managers = typename std::tuple<ComponentArray<Args, N>*...>;
    using func_type_nodes = typename std::tuple<ComponentGroupNode<Args, N>*...>;
    using expr_equiv_groups = typename BoolTupleGenerator<sizeof...(Args)>::type;
    using comp_ptrs = typename std::tuple<Args*...>;

    SystemFunc(CoreCompManager* compMap, SharedResources* resources);

    BoolExprBitVector<N> expr;
    CoreCompManager* m_coreManager;
    SharedResources* _resources;
    void (*decisionFunc)(Base_SystemFunc*);
    func_type_managers managers;
    func_type_nodes tightestGroups;
    comp_ptrs componentPtrs;
    expr_equiv_groups equivs; bool allEquiv, someEquiv;
    Base_ComponentGroupNode<N>* leadingEquivGroupNode = nullptr; // the first group node which is equiv in expression, may be NULL
    Base_ComponentGroupNode<N>* smallestGroup = nullptr;

    void UpdateAfterEntityCommits() override;




    void RunSystem() override {
        decisionFunc(this);
    }


    void RunFullGroups() override;



    template <typename T>
    constexpr static size_t GetIndexInParameterTypeList();

    void RunPartialEquivGroups() override;

    void RunBlindGroups() override;


    bool HasFullEquivGroup() override {
        return allEquiv;
    }
    bool HasPartialEquivGroup() override {
        return !allEquiv && someEquiv;
    }

};
