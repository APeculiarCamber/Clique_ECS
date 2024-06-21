// ECS_src.cpp : This file contains the 'main' function. Program execution begins and ends there.
// 
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <random>
#include <fstream>
#include "SystemImpl.h"
#include "CoreComponentManagerImpl.h"
#include "ECS_Manager.h"
#include "RateChangeBenchmark.h"
#include "ExampleECS.h"

using namespace std;

int RunDiffOpsBenchmark(size_t numItrs, size_t numEntities, std::ofstream& out);

void RunPartialTestings();
int RunTagsBasedBenchmark(size_t numItrs, size_t numEntities, std::ostream& out);

#define RUN_DERIV_BENCH

int main()
{
#ifndef RUN_DERIV_BENCH
    ExampleECS();
#else
    // RunTagsBasedBenchmark(100, 100, std::cout);
    // RunPartialTestings();
    
    // Add group A, B
    // Want to add group C & B
        // We can make a partial using C rooted, then going RIGHT with C & B
        // This would require some adjuctments to traversal
        // 
        // The general would be to create a new group which uses only the groups that don't already have a group tree

    // Open the file "output.txt" for writing
    // std::ofstream outFile("change_test_output.txt");
    std::ofstream outFile("change_test_output_supl.txt");

    // Check if the file was opened successfully
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return 1;
    }

    // TODO : also keep in mind: sparse array vs hash, tup vs no tup, counting install times

    constexpr int shuffle = 1, sort = 2;
    outFile << "{";
    for (size_t numItrs : {1000})
        for (size_t numEntities : {10000, 50000, 100000, 500000, 1000000, 5000000 })
            for (size_t numDerivs : {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}) {
                for (int order : {0, shuffle, sort}) {
                    std::cout << "(ents=" << numEntities << ", derivs=" << numDerivs << ", order='" << (order == 0 ? "inplace" : (order == shuffle ? "shuffle" : "sort")) << "')" << std::endl;

                    outFile << "(" << numEntities << ", " << numDerivs << ", '" << (order == 0 ? "inplace" : (order == shuffle ? "shuffle" : "sort")) << "') : ";
                    Rate::RunChangeTimeBenchmark(numEntities, numItrs, numDerivs, true, order == shuffle, order == sort, outFile);
                }
            }
    outFile << "}" << std::endl;
    outFile.close();
#endif
}






using namespace std;

struct OpsRes {
    size_t res;
};

struct Base_MatMultiplier {
    virtual void ApplyMult(size_t res) = 0;
    virtual size_t ID() = 0;
};

template <size_t N>
struct Mat {
    double a[N][N];
    double b[N][N];
    double c[N][N];
};

template <size_t N>
struct Impl_MatMultiplier : Base_MatMultiplier {
    Mat<N> mat;

    void ApplyMult(size_t res) override {
        double(&a)[N][N] = res == 0 ? mat.b : mat.a;
        double(&b)[N][N] = res == 0 ? mat.c : (res == 1 ? mat.c : mat.b);
        double(&c)[N][N] = res == 0 ? mat.a : (res == 1 ? mat.b : mat.c);

        unsigned i, j, k;
        for (i = 0; i < N; ++i)
            for (j = 0; j < N; ++j)
                for (k = 0; k < N; ++k) {
                    c[i][j] += a[i][k] * b[k][j];
                }

    }

    size_t ID() override {
        return N;
    }
};

template <size_t N>
void MakeMatrixMultNoTup(OpsRes* opres, Mat<N>* mat) {
    size_t res = opres->res;
    double(&a)[N][N] = res == 0 ? mat->b : mat->a;
    double(&b)[N][N] = res == 0 ? mat->c : (res == 1 ? mat->c : mat->b);
    double(&c)[N][N] = res == 0 ? mat->a : (res == 1 ? mat->b : mat->c);

    unsigned i, j, k;
    for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
            for (k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
}

template <size_t N>
void MakeMatrixMult(std::tuple<OpsRes*, Mat<N>*>& data) {
    size_t res = std::get<OpsRes*>(data)->res;
    Mat<N>* mat = std::get<Mat<N>*>(data);
    double(&a)[N][N] = res == 0 ? mat->b : mat->a;
    double(&b)[N][N] = res == 0 ? mat->c : (res == 1 ? mat->c : mat->b);
    double(&c)[N][N] = res == 0 ? mat->a : (res == 1 ? mat->b : mat->c);

    unsigned i, j, k;
    for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
            for (k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
}

struct Grid {
    size_t grid[10][10];
};

struct GridPos {
    int x, y; size_t id;
};

struct MoveRandomTag {};
struct MoveLeftRightTag {};
struct MoveUpDownTag {};

//using TagCompList = typename ECS::ComponentList<GridPos, MoveRandomTag, MoveLeftRightTag, MoveUpDownTag>;
void RunRandomMovement(Grid* grid, size_t* ent, CoreComponentManager<GridPos, MoveRandomTag, MoveLeftRightTag, MoveUpDownTag>* mngr, GridPos* pos) {
    pos->x += (std::rand() % 2 == 0) ? 1 : -1;
    pos->y += (std::rand() % 2 == 0) ? 1 : -1;
    PRINT("RANDOM MOVE " << pos->id << " FOR ENT# " << *ent << ": " << pos->x << ", " << pos->y << std::endl);
    if (pos->id == 0) {
        pos->x = pos->y = 123;
        EntityMeta meta{ (uint32_t)*ent, 1 };
        mngr->DeleteComponents<MoveRandomTag>(meta);
        MoveUpDownTag newTag;
        mngr->AddComponents(meta, &newTag);
    }
}
void RunUpDownMovement(Grid* grid, size_t* ent, CoreComponentManager<GridPos, MoveRandomTag, MoveLeftRightTag, MoveUpDownTag>* mngr, GridPos* pos) {
    pos->y += (std::rand() % 2 == 0) ? 1 : -1;
    PRINT("UPDOWN MOVE " << pos->id << " FOR ENT# " << *ent << ": " << pos->x << ", " << pos->y << std::endl);
}
void RunLeftRightMovement(Grid* grid, size_t* ent, CoreComponentManager<GridPos, MoveRandomTag, MoveLeftRightTag, MoveUpDownTag>* mngr, GridPos* pos) {
    pos->x += (std::rand() % 2 == 0) ? 1 : -1;
    PRINT("LEFTRIGHT MOVE " << pos->id << " FOR ENT# " << *ent << ": " << pos->x << ", " << pos->y << std::endl);
}

int RunTagsBasedBenchmark(size_t numItrs, size_t numEntities, std::ostream& out) {
    ECS::Manager<Grid, ECS::ComponentList<GridPos, MoveRandomTag, MoveLeftRightTag, MoveUpDownTag>> manager;
    manager._componentManager.AddGroup<GContains<GridPos>, GNotContains<>>();
    manager._componentManager.AddGroup<GContains<GridPos, MoveRandomTag>, GNotContains<MoveLeftRightTag, MoveUpDownTag>>();
    manager._componentManager.AddGroup<GContains<GridPos, MoveLeftRightTag>, GNotContains<MoveRandomTag, MoveUpDownTag>>();
    manager._componentManager.AddGroup<GContains<GridPos, MoveUpDownTag>, GNotContains<MoveRandomTag, MoveLeftRightTag>>();
    manager._componentManager.CommitGroups();

    for (size_t i = 0; i < numEntities; ++i) {
        if (i % 3 == 0) {
            GridPos pos{ 0, 0, i }; MoveRandomTag tag;
            manager._componentManager.AddEntity(&pos, &tag);
        }
        else if (i % 3 == 1) {
            GridPos pos{ 0, 0, i }; MoveUpDownTag tag;
            manager._componentManager.AddEntity(&pos, &tag);
        }
        else {
            GridPos pos{ 0, 0, i }; MoveLeftRightTag tag;
            manager._componentManager.AddEntity(&pos, &tag);
        }
    }
    manager._componentManager.CommitEntityChanges();

    auto s = manager.AddSystem<SContains<MoveRandomTag>, SNotContains<MoveLeftRightTag, MoveUpDownTag>, RunRandomMovement>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<MoveLeftRightTag>, SNotContains<MoveRandomTag, MoveUpDownTag>, RunLeftRightMovement>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<MoveUpDownTag>, SNotContains<MoveRandomTag, MoveLeftRightTag>, RunUpDownMovement>();
    assert(s->HasFullEquivGroup());

    manager.SetTickLimit(2);
    manager.RunSystems();

    return 0;
}

int RunDiffOpsBenchmark(size_t numItrs, size_t numEntities, std::ofstream& out) {

    ECS::Manager <OpsRes, ECS::ComponentList<Mat<2>, Mat<3>, Mat<4>, Mat<5>, Mat<6>>> manager;
    manager.GetSharedResources()->res = 2;
    manager._componentManager.CommitGroups();

    std::vector<Base_MatMultiplier*> ptrMatVec;
    // Entities
    std::srand((unsigned int)std::time(nullptr));
    for (size_t i = 0; i < numEntities; ++i) {
        size_t r = std::abs(std::rand()) % 7;
        switch (r) {
        case 0:
            Mat<2> mat2;
            manager._componentManager.AddEntity(&mat2);
            ptrMatVec.push_back(new Impl_MatMultiplier<2>());
            break;
        case 1:
            Mat<3> mat3;
            manager._componentManager.AddEntity(&mat3);
            ptrMatVec.push_back(new Impl_MatMultiplier<3>());
            break;
        case 2:
            Mat<4> mat4;
            manager._componentManager.AddEntity(&mat4);
            ptrMatVec.push_back(new Impl_MatMultiplier<4>());
            break;
        case 3:
            Mat<5> mat5;
            manager._componentManager.AddEntity(&mat5);
            ptrMatVec.push_back(new Impl_MatMultiplier<5>());
            break;
        case 4:
            Mat<6> mat6;
            manager._componentManager.AddEntity(&mat6);
            ptrMatVec.push_back(new Impl_MatMultiplier<6>());
            break;
        default: break;
        }
    }
    clock_t commitStart = std::clock();
    manager._componentManager.CommitEntityChanges();
    clock_t commitEnd = std::clock();
    out << "IT TOOK " << (commitEnd - commitStart) << " TO COMMIT THE ENTITIES" << std::endl;
    auto s = manager.AddSystem<SContains<>, SNotContains<>, MakeMatrixMult<2>>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<>, SNotContains<>, MakeMatrixMult<3>>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<>, SNotContains<>, MakeMatrixMult<4>>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<>, SNotContains<>, MakeMatrixMult<5>>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<>, SNotContains<>, MakeMatrixMult<6>>();
    assert(s->HasFullEquivGroup());
    manager.AddCleanupSystem([] (OpsRes* res, decltype(manager)* manager) {
        res->res = (res->res + 1) % 3;
    });

    // std::shuffle(ptrMatVec.begin(), ptrMatVec.end(), std::default_random_engine());

    commitStart = std::clock();
    std::sort(ptrMatVec.begin(), ptrMatVec.end(), [](Base_MatMultiplier* p1, Base_MatMultiplier* p2) {return p1->ID() < p2->ID(); });
    commitEnd = std::clock();
    out << "IT TOOK " << (commitEnd - commitStart) << " TO SORT THE VIRTUAL POINTER ARRAY" << std::endl;

    out << "RUNNING ON THE MAT MULT OPS BENCHMARK, with numItrs=" << numItrs << " and numEntities=" << numEntities << std::endl;
    manager.SetTickLimit(numItrs);
    clock_t ecsstart = std::clock();
    manager.RunSystems();
    out << "--- ECS FINISH IN: " << ((float)(std::clock() - ecsstart) / (float)(CLOCKS_PER_SEC / 1000)) << " MILLISECONDS" << std::endl;


    clock_t start = std::clock();
    clock_t lastFrame = start;
    size_t res = 0;
    for (size_t i = 0; i < numItrs; ++i) {
        for (size_t p = 0; p < ptrMatVec.size(); ++p) {
            ptrMatVec[p]->ApplyMult(res);
        }

        res = (res + 1) % 3;
    }
    out << "--- PTR FINISH IN: " << ((float)(std::clock() - start) / (float)(CLOCKS_PER_SEC / 1000)) << " MILLISECONDS" << std::endl << std::endl;



    // Clean up
    for (size_t p = 0; p < ptrMatVec.size(); ++p) {
        delete ptrMatVec[p];
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file












struct A { size_t x;  };
struct B { size_t x;  };
struct C { size_t x;  };
struct ResABC {};

class Base_Guy {
public:
    virtual void Run() = 0;
};

template <typename X, typename Y>
class RunTestGuy : public Base_Guy {
public:
    X x;
    Y y;

    void Run() override {
        x.x *= y.x;
    }
};
class RunTestGuyABC : public Base_Guy {
public:
    A x;
    B y;
    C z;

    void Run() override {
        x.x *= y.x + z.x;
    }
};


template <typename X, typename Y>
class RunPTRGuy : public Base_Guy {
public:
    RunPTRGuy(X* xx, Y* yy) : x(xx), y(yy) {}

    X* x;
    Y* y;

    void Run() override {
        x->x *= y->x;
    }
};

template <typename X, typename Y>
void RunSystem(X* x, Y* y) {
    // assert(x->y == y->y);
    x->x *= y->x;
}

void RunPartialTestings() {
    ECS::Manager<ResABC, ECS::ComponentList<A, B, C>> manager;
    char abType = manager._componentManager.AddGroup<GContains<A, B>, GNotContains<>>();
    assert(abType == EXPLICIT_ADD);
    char bcType = manager._componentManager.AddGroup<GContains<B, C>, GNotContains<>>(true, true);
    assert(bcType == IMPLICIT_ADD);
    manager._componentManager.CommitGroups();

    auto s = manager.AddSystem<SContains<>, SNotContains<>, RunSystem<A, B>>();
    assert(s->HasFullEquivGroup());
    s = manager.AddSystem<SContains<>, SNotContains<>, RunSystem<B, C>>();
    assert(s->HasPartialEquivGroup());

    constexpr size_t numEntities = 1000000;
    std::vector<A> As;
    As.reserve(numEntities);
    std::vector<B> Bs;
    Bs.reserve(numEntities);
    std::vector<C> Cs;
    Cs.reserve(numEntities);

    std::vector<Base_Guy*> guys;
    std::vector<Base_Guy*> ptrGuys;
    for (size_t i = 0; i < numEntities; i++) {
        if (i % 3 == 0) {
            A a{ i }; B b{ i };
            manager._componentManager.AddEntity(&a, &b);
            guys.push_back(new RunTestGuy<A, B>());
            As.emplace_back(); Bs.emplace_back();
            ptrGuys.push_back(new RunPTRGuy(&As.back(), &Bs.back()));
        }
        else if (i % 3 == 1) {
            C a{ i }; B b{ i };
            manager._componentManager.AddEntity(&a, &b);
            guys.push_back(new RunTestGuy<B, C>());
            Cs.emplace_back(); Bs.emplace_back();
            ptrGuys.push_back(new RunPTRGuy(&Bs.back(), &Cs.back()));
        }
        else {
            A c{ i }; C a{ i }; B b{ i };
            manager._componentManager.AddEntity(&c, &a, &b);
            guys.push_back(new RunTestGuy<A, B>());
            guys.push_back(new RunTestGuy<B, C>());
            As.emplace_back(); Bs.emplace_back(); Cs.emplace_back();
            ptrGuys.push_back(new RunPTRGuy(&As.back(), &Bs.back()));
            ptrGuys.push_back(new RunPTRGuy(&Bs.back(), &Cs.back()));
        }
    }
    manager.CommitEntityChanges();

    std::clock_t start = std::clock();
    manager.SetTickLimit(10);
    manager.RunSystems();
    std::cout << "ECS Done in " << (std::clock() - start) << std::endl;


    start = std::clock();
    for (size_t i = 0; i < 10; ++i) {
        for (auto* g : guys) {
            g->Run();
        }
    }
    std::cout << "PTR Done in " << (std::clock() - start) << std::endl;


    start = std::clock();
    for (size_t i = 0; i < 10; ++i) {
        for (auto* g : ptrGuys) {
            g->Run();
        }
    }
    std::cout << "PTR-by-PTR Done in " << (std::clock() - start) << std::endl;
}













#ifdef GRID_BASE
#define GRID_SIZE 20

struct TestSharedResources {
    std::clock_t lastFrame;
    size_t bounces = 0;
    float deltaTime = 0;
    float minx, miny, minz, maxx, maxy, maxz;
    char grid[GRID_SIZE][GRID_SIZE + 1];
};

int GridBenchmarkOrSomething() {
    // Create an unordered_map of void* pointers to Abs objects with keys based on the argument types of `foo`

    // BIGGEST TODOs:
        // - make sure ALL works, big task
        // mark groups with the components they acceptably allow to iterate over

    ECS::Manager<TestSharedResources, ECS::ComponentList<Position, Velocity, Acceleration, Jerk, SafeJerk> > manager;
    auto res = manager.GetSharedResources();
    res->minx = res->miny = res->minz = 0;
    res->maxx = res->maxy = res->maxz = 10;
    res->lastFrame = std::clock();
    res->bounces = 0;
    res->deltaTime = 0;


    // TODO : the system you have now will collapse on this, but you should think about if it actually NEEDS to...
        // I mean obviously we can fix this with system MUST HAVEs
    manager._componentManager.AddGroup<GContains<Position, Velocity>, GNotContains<>>();
    manager._componentManager.AddGroup<GContains<Position, Velocity, Acceleration>, GNotContains<>>();
    manager._componentManager.AddGroup<GContains<Position, Velocity, Acceleration, Jerk>, GNotContains<>>();
    manager._componentManager.AddGroup<GContains<Position, Velocity, Acceleration, Jerk>, GNotContains<SafeJerk>>();
    manager._componentManager.CommitGroups();

    for (size_t i = 0; i < 10; ++i) {
        Velocity vel{ (rand() / (float)RAND_MAX) - 0.5f, (rand() / (float)RAND_MAX) - 0.5f, (rand() / (float)RAND_MAX) - 0.5f };
        Position pos{ (rand() / (float)RAND_MAX) * 10.0f, (rand() / (float)RAND_MAX) * 10.0f, (rand() / (float)RAND_MAX) * 10.0f };
        manager._componentManager.AddEntity(&pos, &vel);
    }

    for (size_t i = 0; i < 12; ++i) {
        Velocity vel{ (rand() / (float)RAND_MAX) - 0.5f, (rand() / (float)RAND_MAX) - 0.5f, (rand() / (float)RAND_MAX) - 0.5f };
        Position pos{ (rand() / (float)RAND_MAX) * 10.0f, (rand() / (float)RAND_MAX) * 10.0f, (rand() / (float)RAND_MAX) * 10.0f };
        Acceleration acc{ i, i, i };
        manager._componentManager.AddEntity(&pos, &vel, &acc);
    }

    manager._componentManager.CommitEntityChanges();
    Velocity vel{ 123, 123, 123 };
    Position pos{ 123, 123, 123 };
    Acceleration acc{ 123, 123, 123 };
    EntityMeta ent = manager._componentManager.AddEntity(&pos, &vel, &acc);

    manager._componentManager.CommitEntityChanges();

    manager._componentManager.DeleteEntity(ent._handler, ent._uniqueID);

    manager._componentManager.CommitEntityChanges();

    auto s = manager.AddSystem<SContains<>, SNotContains<>>([](TestSharedResources* res, Position* pos, Velocity* vel) {
        bool forceUp = pos->z <= res->minz && (vel->z < 0);
        bool forceRight = pos->y <= res->miny && (vel->y < 0);
        bool forceForw = pos->x <= res->minx && (vel->x < 0);
        bool forceDown = pos->z >= res->maxz && (vel->z > 0);
        bool forceLeft = pos->y >= res->maxy && (vel->y > 0);
        bool forceBack = pos->x >= res->maxx && (vel->x > 0);
        res->bounces += (forceUp + forceRight + forceForw + forceDown + forceLeft + forceBack);

        vel->z *= (forceUp ? -1 : 1) * (forceDown ? -1 : 1);
        vel->y *= (forceRight ? -1 : 1) * (forceLeft ? -1 : 1);
        vel->x *= (forceForw ? -1 : 1) * (forceBack ? -1 : 1);

        pos->x += vel->x * res->deltaTime;
        pos->y += vel->y * res->deltaTime;
        pos->z += vel->z * res->deltaTime;

        if (forceUp + forceRight + forceForw + forceDown + forceLeft + forceBack) {
            std::cout << "Bounce #" << res->bounces << " at Pos={" << pos->x << "," << pos->y << "," << pos->z << "}" << std::endl;
        }
        });


    s = manager.AddSystem<SContains<>, SNotContains<>>([](TestSharedResources* res, Position* pos) {
        float x = std::min(0.99f, std::max(0.0f, (pos->x - res->minx) / (res->maxx - res->minx)));
        float y = std::min(0.99f, std::max(0.0f, (pos->y - res->miny) / (res->maxy - res->miny)));
        float z = std::min(0.99f, std::max(0.0f, (pos->z - res->minz) / (res->maxz - res->minz)));

        size_t ix = (GRID_SIZE * x), iy = (GRID_SIZE * y), iz = (GRID_SIZE * z);
        res->grid[ix][iy] = z > 0.5f ? '\u2588' : '*';

        });
    assert(s->HasFullEquivGroup());

    // UPDATE THE DELTA TIME & CLOSE THE SYSTEM IF WE REACH THE TARGET BOUNCE AMOUNT
    constexpr size_t BOUNCE_LIMIT = 100;
    manager.AddCleanupSystem([BOUNCE_LIMIT](TestSharedResources* res, decltype(manager)* manager) {
        std::system("cls"); // for Windows

        std::clock_t endOfFrame = std::clock();
        res->deltaTime = (float)(endOfFrame - res->lastFrame) / (float)CLOCKS_PER_SEC;
        res->lastFrame = endOfFrame;
        manager->_running &= res->bounces < BOUNCE_LIMIT;


        for (size_t g = 0; g < GRID_SIZE; ++g) {
            res->grid[g][GRID_SIZE] = '\0';
            std::cout << res->grid[g] << '\n';
            std::fill(res->grid[g], res->grid[g] + GRID_SIZE, ' ');
        }
        std::cout << std::endl;

        });
    /*
    s = manager.AddSystem<SContains<Position>, SNotContains<>>([deltaTime](Velocity* pos, Acceleration* vel) {
        pos->x += vel->x * deltaTime;
        pos->y += vel->y * deltaTime;
        pos->z += vel->z * deltaTime;
        });
    assert(s->HasFullEquivGroup());

    s = manager.AddSystem<SContains<Position, Velocity>, SNotContains<>>([deltaTime](Acceleration* pos, Jerk* vel) {
        pos->x += vel->x * deltaTime;
        pos->y += vel->y * deltaTime;
        pos->z += vel->z * deltaTime;
        });
    assert(s->HasFullEquivGroup());

    s = manager.AddSystem<SContains<Position, Velocity, Acceleration>, SNotContains<SafeJerk>>([deltaTime](Jerk* jerk) {
        jerk->x += (float)rand() / 1100000.0f;
        jerk->y += (float)rand() / 1032000.0f;
        jerk->z += (float)rand() / 1005600.0f;
        });
    assert(s->HasFullEquivGroup());
    */
    // manager.SetTickLimit(10);
    manager.RunSystems();
    std::cout << std::endl;



    //manager.AddSystem<SContains<int, double>, SNotContains<char, bool>>(foo2);

    //return 0;
    //CoreComponentManager<Foo11, Foo22222, Pog> mainManager;

    // mainManager.GetManagerMap();

    //SystemFunc<CoreComponentManager<Foo11, Foo22222, Pog>, SContains<int, double>, SNotContains<char, bool>, Foo11, Foo22222, Pog> t(foo2, mainManager);
    //t.RunSystem();


    //Pog pog;
    //mainManager.AddComponents<Pog>({ 0, 0 }, &pog);

    /*
    for (auto x : abs_map)
        std::cout << x.second << ", " << std::endl;

    // Create a tuple of Abs objects with template parameters based on the argument types of `foo`
    using arg_types = typename FunctionTraits<decltype(&foo)>::abs_arg_tuple;
    arg_types abs_funcs;


    // Extract the template parameter type of each Abs object using partial template specialization and type traits
    std::apply([](auto... abs_ptrs) {
        ((std::cout << typeid(typename meta::underlying_type<decltype(abs_ptrs)>::type).name() << std::endl), ...);
        }, abs_funcs);

    // Copy the Abs objects from the unordered_map to the tuple
    std::apply([&abs_map, &abs_funcs](auto&... args) {
        ((args = static_cast<typename std::decay<decltype(args)>::type>(abs_map[std::type_index(typeid(typename meta::underlying_type<std::decay_t<decltype(args)>>::type))])), ...);
        ((std::cout << abs_map[std::type_index(typeid(typename meta::underlying_type<std::decay_t<decltype(args)>>::type))] << ", " << std::endl), ...);
        }, abs_funcs);

    // Apply each Abs object to its corresponding argument and print the results
    // std::cout << std::apply([](auto... args) { return foo((*args)()...); }, abs_funcs) << std::endl;
    */
}
#endif
