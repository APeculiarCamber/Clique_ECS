
#ifndef SYSTEM_IMPL_H
#define SYSTEM_IMPL_H


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


void System_RunFullGroups(Base_SystemFunc *func) {
    func->RunFullGroups();
}

void System_RunPartialGroups(Base_SystemFunc *func) {
    func->RunPartialEquivGroups();
}

void System_RunBlindGroups(Base_SystemFunc *func) {
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



#define SYSTEM_TEMPLATE template<typename... AllCs, class SharedResources, class... ConCs, class... NotConCs, typename... Args, typename... FullArgs, typename FuncTraits, auto func>
#define SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func) CoreComponentManager<AllCs...>, SharedResources, SContains<ConCs...>, SNotContains<NotConCs...>, FuncArgs<Args...>, FuncArgs<FullArgs...>, FuncTraits, func
SYSTEM_TEMPLATE
SystemFunc<SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func)>::SystemFunc(
        CoreComponentManager<AllCs...> *compMap, SharedResources *resources) : m_coreManager(compMap), _resources(resources)
{
    auto has = std::tuple_cat(
            std::make_tuple((size_t)compMap->template getTypeIndex<ConCs>()...),
            std::make_tuple((size_t)compMap->template getTypeIndex<Args>()...));
    auto notHas = std::make_tuple((size_t)compMap->template getTypeIndex<NotConCs>()...);
    expr = MakeFromTupleSpec<decltype(has), decltype(notHas), N>(has, notHas);
    DEBUG_PRINT_SYSTEM_SIGNATURE(has, notHas);


    // Extract the template parameter type of each manager object using partial template specialization and type traits
    // Get the pointers to managers from the compMap and set all the manager classes.
    // Each args is of type ComponentArray<T, N>, so we use meta::underlying_type to extract the T.
    std::apply([compMap](auto&... args) {
        ((args = static_cast<typename std::decay<decltype(args)>::type>(compMap->template
                GetComponentArray<typename meta::underlying_type<std::decay_t<decltype(args)>>::type>())), ...);
        PRINT_FOLD(typeid(typename meta::underlying_type<std::decay_t<decltype(args)>>::type).name() << ", " << args->TypeName() << std::endl);
    }, managers);

    // TOPORT: see if we can't constexpr some of this
    // Get the best group based on expression
    tightestGroups = std::make_tuple(std::get<ComponentArray<Args, N>*>(managers)->GetBestGroup(expr)...);
    // Assign true to tuple of booleans if corresponding group has exactly equivalent (<-->) expression to this system's
    // TAKE CARE: this assumes that ordering of Args... is maintained, this assumption is used by Partial Group systems
    equivs = std::make_tuple(std::get<ComponentGroupNode<Args, N>*>(tightestGroups)->IsEquivalent(expr)...);

    allEquiv = std::apply([](auto... args) { return (args && ...); }, equivs);

    PRINT(allEquiv << std::endl);

    // Determine leading Group node if it exists
    leadingEquivGroupNode = nullptr;
    std::apply([&](auto... args) {
        ((leadingEquivGroupNode = ((args->IsEquivalent(expr)
                                    && ((!leadingEquivGroupNode) || (args->GetNumShared() > leadingEquivGroupNode->GetNumShared()))))
                                  ? static_cast<Base_ComponentGroupNode<N>*>(args) : leadingEquivGroupNode), ...);
    }, tightestGroups);
    someEquiv = leadingEquivGroupNode != nullptr;

    // If the system registers as 'tight' with a group, print it out
    if (auto groupQ = std::get<0>(tightestGroups); groupQ)
        DEBUG_PRINT_TIGHT_SYSTEM(expr, groupQ);

    // Determine the group with the smallest number of components,
    smallestGroup = static_cast<Base_ComponentGroupNode<N>*>(std::get<0>(tightestGroups));
    std::apply([&](auto... args) {
        ((smallestGroup = (smallestGroup->GetNumComponents() > args->GetNumComponents())
                          ? static_cast<Base_ComponentGroupNode<N>*>(args) : smallestGroup), ...);
    }, tightestGroups);

    // Select the function pointer which we will use to dispatch our system execution
    if (allEquiv)
        decisionFunc = System_RunFullGroups;
    else if (leadingEquivGroupNode)
        decisionFunc = System_RunPartialGroups;
    else // has only blind groups
        decisionFunc = System_RunBlindGroups;
}





SYSTEM_TEMPLATE
void SystemFunc<SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func)>::UpdateAfterEntityCommits() {
    smallestGroup = static_cast<Base_ComponentGroupNode<N>*>(std::get<0>(tightestGroups));
    std::apply([&](auto... args) {
        ((smallestGroup = (smallestGroup->GetNumComponents() > args->GetNumComponents())
                          ? static_cast<Base_ComponentGroupNode<N>*>(args) : smallestGroup), ...);
    }, tightestGroups);

    componentPtrs = std::make_tuple(std::get<ComponentGroupNode<Args, N>*>(tightestGroups)->GetStart()...);
}

SYSTEM_TEMPLATE
void SystemFunc<SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func)>::RunFullGroups() {
    size_t num = std::get<0>(tightestGroups)->GetNumComponents();
    PRINT("RUNNING FULL GROUP WITH " << num << std::endl);

    std::tuple<FullArgs*...> current;
    if constexpr (FuncTraits::UsesRes) {
        if constexpr (FuncTraits::UsesEntityID)
            current = std::tuple_cat(std::make_tuple(_resources, leadingEquivGroupNode->GetStartEntityIndPtr(), m_coreManager), componentPtrs);
        else
            current = std::tuple_cat(std::make_tuple(_resources), componentPtrs);
    }
    else {
        current = componentPtrs;
    }

    for (size_t i = 0; i < num; ++i) {
        if constexpr (FuncTraits::UsesTuple)
            func(current);
        else
            func(std::get<FullArgs*>(current)...);

        ((std::get<Args*>(current)++), ...);

        if constexpr (FuncTraits::UsesEntityID)
            std::get<size_t*>(current)++;
    }
    PRINT("FULL GROUP DONE" << std::endl);
}

SYSTEM_TEMPLATE
void SystemFunc<SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func)>::RunPartialEquivGroups() {
    size_t num = leadingEquivGroupNode->GetNumComponents();
    PRINT("PARTIAL GROUP RUNNING WITH " << num << std::endl);

    size_t* entInd = leadingEquivGroupNode->GetStartEntityIndPtr();

    std::tuple<FullArgs*...> current;
    if constexpr (FuncTraits::UsesRes) {
        if constexpr (FuncTraits::UsesEntityID)
            current = std::tuple_cat(std::make_tuple(_resources, entInd, m_coreManager), componentPtrs);
        else
            current = std::tuple_cat(std::make_tuple(_resources), componentPtrs);
    }
    else {
        current = componentPtrs;
    }


    for (size_t i = 0; i < num; ++i) {
        ((std::get<Args*>(current) = std::get<GetIndexInParameterTypeList<Args>()>(equivs)
                                     ? std::get<Args*>(current)
                                     : std::get<ComponentArray<Args, N>*>(managers)->GetComponentForEntity(*entInd)), ...);

        if constexpr (FuncTraits::UsesTuple)
            func(current);
        else
            func(std::get<FullArgs*>(current)...);

        ((std::get<Args*>(current)++), ...);
        entInd++;

        if constexpr (FuncTraits::UsesEntityID)
            std::get<size_t*>(current) = entInd;
    }
    PRINT("PARTIAL GROUP DONE" << std::endl);

}

SYSTEM_TEMPLATE
void SystemFunc<SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func)>::RunBlindGroups() {
    size_t possibleNum = smallestGroup->GetNumComponents();
    size_t* entInd = smallestGroup->GetStartEntityIndPtr();

    std::tuple<FullArgs*...> current;
    if constexpr (FuncTraits::UsesRes) {
        if constexpr (FuncTraits::UsesEntityID)
            current = std::tuple_cat(std::make_tuple(_resources, smallestGroup->GetStartEntityIndPtr(), m_coreManager), componentPtrs);
        else
            current = std::tuple_cat(std::make_tuple(_resources), componentPtrs);
    }
    else {
        current = componentPtrs;
    }


    for (size_t i = 0; i < possibleNum; ++i) {
        if (m_coreManager->EntityValidForSystem(*entInd, expr)) {
            ((std::get<Args*>(current) = std::get<ComponentArray<Args, N>*>(managers)->GetComponentForEntity(*entInd)), ...);

            if constexpr (FuncTraits::UsesTuple)
                func(current);
            else
                func(std::get<FullArgs*>(current)...);
        }

        entInd++;
        if constexpr (FuncTraits::UsesEntityID)
            std::get<size_t*>(current)++;
    }
}



SYSTEM_TEMPLATE template<typename T>
constexpr size_t SystemFunc<SYSTEM_EXTRACT(AllCs, ConCs, NonConCs, Args, FullArgs, FuncTraits, func)>::GetIndexInParameterTypeList() {
    size_t index = 0;
    size_t id = 0;
    ((id = std::is_same_v<T, Args> ? index : ((index++) & 0) + id), ...); // really dumb clever way to count on failure until success in a variadic operation list
    return id;
}


#endif