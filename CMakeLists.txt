cmake_minimum_required(VERSION 3.28)
project(Clique_ECS)

set(CMAKE_CXX_STANDARD 14)

include_directories(ECS_src)

add_executable(Clique_ECS
        ECS_src/BooleanExprTree.h
        ECS_src/BoolExprBitVector.h
        ECS_src/BoolExprTreeManager.h
        ECS_src/ComponentArray.h
        ECS_src/ComponentSparseSet.h
        ECS_src/CoreComponentManagerImpl.h
        ECS_src/ECS_AutoGrouper.cpp
        ECS_src/ECS_Manager.h
        ECS_src/EntityImpl.h
        ECS_src/ExampleECS.h
        ECS_src/PrintDebugger.h
        ECS_src/RateChangeBenchmark.h
        ECS_src/SystemImpl.h)
