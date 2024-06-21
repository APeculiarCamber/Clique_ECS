#pragma once
#include "ECS_Manager.h"


// Not mentioned in the paper: a resource object represents global state for an ECS, 
// it is accessible to every system, and would require strict syncronization primitives in parallel ECSs
struct ResourceObject {
	std::ofstream out;
};

// A component
struct A_Comp {
	char op;
};
// B component
struct B_Comp {
	float val;
};

// System functions MUST use component pointers

// System functions can use a tuple to concat components together, 
// passing a tuple reference was faster than using multiple pointer parameters in early tests
__forceinline void RunAB(std::tuple<A_Comp*, B_Comp*>& ab) {
	A_Comp* a = std::get<A_Comp*>(ab);
	B_Comp* b = std::get<B_Comp*>(ab);
	b->val = (a->op == 'A') ? (b->val + b->val) : (b->val * b->val);
}
// System functions can also simply use component pointers directly
void RunA(A_Comp* a) {
	a->op = (std::rand() % 2) == 0 ? 'A' : 'M';
}

// System functions can also use a pointer to the global resource object
void RunB(ResourceObject* res, B_Comp* b) {
	res->out << b << ", ";
}

void ExampleECS() {
	// Managers first take a single template parameter which represents the 'resource' for the ECS; this can be thought of as the global state.
	// Next, the list of all possible component types are passed in as a template pack, wrapped in an ECS::ComponentList.
	// Do NOT miss a single component type. This will cause severe errors, since the component type will not correctly get a unique type index.
	ECS::Manager<ResourceObject, ECS::ComponentList<A_Comp, B_Comp>> manager;

	manager.GetSharedResources()->out.open("ExampleOutFile.txt");

	// Groups are created using template packs wrapped in GContains and GNotContains structs, which define their component expression/signature.
	manager.MakeGroup<GContains<A_Comp, B_Comp>, GNotContains<>>();

	// Entities should be added AFTER all groups.
	// We create groups by providing a parameter list of pointers for each component the entity will have initially.
	A_Comp a{ 'A' };
	B_Comp b{ 1 };
	EntityMeta ent = manager.AddEntity(&a, &b);

	// Systems should be added AFTER all groups. Order does not matter between entities and systems however.
	// Similar to groups, the system's component signature is specified by template packs wrapped in SContains and SNotContains.
	// Most importantly, Adding a system also requires providing a void function with one of the following signatures:
		// void(Cs*... cmps), void(Res*, Cs*... cmps), void(std::tuple<Cs*...> cmps), void(std::tuple<Res*, Cs*...> cmps).
		// where Cs is a list of component types, this Cs list of component types is concatenated to the specified SContains type pack for the component signature.
	// The function will be called for each entity which satisfies its signature. 
	Base_SystemFunc* sys = manager.AddSystem<SContains<A_Comp, B_Comp>, SNotContains<>, RunAB>();
	// AddSystem returns a virtual pointer to the created system, which can be queried for properties such as whether it has all Equivalent groups.
	sys->AssertFullEquiv();
	manager.AddSystem<SContains<>, SNotContains<>, RunA>()->AssertFullEquiv();
	manager.AddSystem<SContains<>, SNotContains<>, RunB>()->AssertFullEquiv();

	// Cleanup functions are called at the end of each tick, just before committing entity changes.
	//  It is likely best to specify them as lambda function, while systems should be specified as function pointers, since they can better be inlined.
	//  This is a VERY simple cleanup function which simply prints the number of iterations left.
	//  A cleanup system could also be used to specify more advanced shutdown logic than the run-timer.
	auto cleanup_func = [](ResourceObject* res, decltype(manager)* manager) {
		std::cout << "Current Tick: " << manager->_currentRunCount << std::endl;

		res->out << std::endl; // end line for the outfile
	};
	// Add the cleanup system
	manager.AddCleanupSystem(cleanup_func);

	// The tick limit is the number of ticks which the ECS will run before stopping.
	// If the tick limit is set to 0 then the ECS will run indefinitely, OR until a cleanup system shuts down the 
	manager.SetTickLimit(100);

	// This function finalizes all setup and begins running the systems. 
	// No changes can be made after calling this function.
	manager.RunSystems();

	// That's all folks...
	manager.GetSharedResources()->out.close();
}