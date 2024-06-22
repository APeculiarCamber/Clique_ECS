#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "ECS_Manager.h"
#include <fstream>
#include <filesystem>

#define UNIFORM_RAND() (float)(std::rand()) / (float)RAND_MAX

// Not mentioned in the paper: a resource object represents global state for an ECS.
// It is accessible to every system (that wants it), and would require strict synchronization primitives in parallel ECSs.
struct ResourceObject {
    std::ofstream out;
    float deltaTime;
    std::clock_t lastFrame;
};


// Our position component
struct Position {
    float x, y, z;
};
// Our velocity component
struct DynamicVelocity {
    float x, y, z;
};

// System functions MUST use component pointers.
// References are possible (they may even be faster) and are theoretically easy to implement, however.
// System functions can use a pointer to the global resource object, but this is not required.

#define GRAVITY 9.8f
void ApplyGravity(ResourceObject* res, DynamicVelocity* vel) {
    vel->y -= (GRAVITY * res->deltaTime);
}

void OutputPositions(ResourceObject* res, Position* pos) {
    res->out << "(x=" << pos->x << ",y=" << pos->y << ",z=" << pos->z << "), ";
}

// System functions can also use a tuple to concat components together.
// Naive benchmarking found this to be slightly faster.
void ApplyVelocity(std::tuple<ResourceObject*, Position*, DynamicVelocity*>& ab) {
    Position& pos = *std::get<Position*>(ab);
    const DynamicVelocity& vel = *std::get<DynamicVelocity*>(ab);
    const ResourceObject* res = std::get<ResourceObject*>(ab);
    pos.x += vel.x * res->deltaTime;
    pos.y += vel.y * res->deltaTime;
    pos.z += vel.z * res->deltaTime;
}



// System can also exclude the resource object if it is unneeded. Like this UNUSED wrapping system for the position.
float WrapIn200x200x200Box(float val) {
    float BOX_MIN = -100, BOX_MAX = 100;
    float boxRange = BOX_MAX - BOX_MIN;
    float nudge = std::ceil(-std::min(0.0f, val / boxRange));
    return std::fmod((val + (nudge * boxRange) - BOX_MIN), boxRange) + BOX_MIN;
}
void WrapPosition(Position* pos) {
    pos->x = WrapIn200x200x200Box(pos->x);
    pos->y = WrapIn200x200x200Box(pos->y);
    pos->z = WrapIn200x200x200Box(pos->z);
}


void ExampleECS() {
    const char* outFilename = "ExampleOutFile.txt";
    // Managers first take a single template parameter which represents the 'resource' for the ECS; this can be thought of as the global state.
    // Next, the list of all possible component types are passed in as a template pack, wrapped in an ECS::ComponentList.
    // Do NOT miss a single component type. This will cause classic template errors, since the component type will not correctly get a unique type index.
    ECS::Manager<ResourceObject, ECS::ComponentList<Position, DynamicVelocity>> manager;
    auto* res = manager.GetSharedResources();
    res->out.open(outFilename); // This is our global state, a file to write to.
    res->lastFrame = std::clock();
    res->deltaTime = 0;

    // Groups are created using template packs wrapped in GContains and GNotContains structs, which define their component expression/signature.
    // If a system has a 'tight' group, then it will be processed completely sequentially in memory, no pointer indirection nor branches.
    manager.MakeGroup<GContains<Position, DynamicVelocity>, GNotContains<>>();

    // Entities should be added AFTER all groups.
    // We create groups by providing a parameter list of pointers for each component the entity will have initially.
    size_t numEntities = 100;
    for (int i = 0; i < numEntities; i++) {
        Position pos{ UNIFORM_RAND() * 10, UNIFORM_RAND() * 10, UNIFORM_RAND() * 10 };
        DynamicVelocity vel{ UNIFORM_RAND() * 15, UNIFORM_RAND() * 30, UNIFORM_RAND() * 15 };
        // AddEntity copies components, there is no scope worry.
        EntityMeta ent = manager.AddEntity(&pos, &vel);
    }

    // Systems should be added AFTER all groups. Order does not matter between entities and systems, however.
    // Similar to groups, the system's component signature is specified by template packs wrapped in SContains and SNotContains.
    // Most importantly, Adding a system also requires providing a void function with one of the following signatures:
    // void(Cs*... cmps), void(Res*, Cs*... cmps), void(std::tuple<Cs*...> cmps), void(std::tuple<Res*, Cs*...> cmps).
    // where Cs is a list of component types, this Cs list of component types is concatenated to the specified SContains type pack for the component signature.
    // The function will be called for each entity which satisfies its signature.
    Base_SystemFunc* sys = manager.AddSystem<SContains<Position, DynamicVelocity>, SNotContains<>, ApplyVelocity>();
    // AddSystem returns a virtual pointer to the created system, which can be queried for properties such as whether it has all Equivalent groups.
    sys->AssertFullEquiv();

    // The arguments of the provided function to an AddSystem call will be implicitly included in SContains.
    // But it's usually good to include them anyway for clarity.
    manager.AddSystem<SContains<>, SNotContains<>, ApplyGravity>()->AssertFullEquiv();
    manager.AddSystem<SContains<>, SNotContains<>, OutputPositions>()->AssertFullEquiv();

    // Cleanup functions are called at the end of each tick, just before committing entity changes (and after ALL run systems).
    //  It is likely best to specify them as lambda function, while systems are given as template function pointers.
    //  This is a VERY simple cleanup function which simply prints the number of iterations left and updates a delta time.
    auto cleanup_func = [](ResourceObject* res, decltype(manager)* manager) {
        // We can write results:
        std::cout << "\nTick #" << manager->GetCurrentRunCount() << ":" << std::endl;
        res->out << std::endl; // end line for the outfile
        // Or manage global state:
        std::clock_t endOfFrame = std::clock();
        res->deltaTime = (float)(endOfFrame - res->lastFrame) / (float)CLOCKS_PER_SEC;
        res->lastFrame = endOfFrame;
        // Or prematurely end the ECS:
        // manager->_running = false;
    };
    // Add the cleanup system. Unlike systems, 'cleanup' signature is fixed and doesn't need to be given a template parameter.
    manager.AddCleanupSystem(cleanup_func);

    // The tick limit is the number of ticks which the ECS will run before stopping.
    // If the tick limit is set to 0 then the ECS will run indefinitely, OR until a cleanup system shuts down the ECS
    manager.SetTickLimit(100);

    // This function finalizes all setup and begins running the systems. It is blocking for this single thread design.
    // No changes to the system or groups can be made after calling this function (but components and entities can be).
    manager.RunSystems();

    // That's all folks...
    manager.GetSharedResources()->out.close();

    std::cout << "Wrote output file to " << std::filesystem::current_path() / outFilename << std::endl;
}

#endif