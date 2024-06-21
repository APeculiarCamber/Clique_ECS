cmake_minimum_required(VERSION 3.28)
project(Clique_ECS)

set(CMAKE_CXX_STANDARD 23)

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
        ECS_src/ExampleECS.h
        ECS_src/RateChangeBenchmark.h
        ECS_src/SystemImpl.h
        ECS_src/BooleanExprTree.cpp
        ECS_src/BooleanExprTree.cpp
        ECS_src/BooleanExprTree.h
        ECS_src/BoolExprBitVector.cpp
        ECS_src/BoolExprBitVector.h
        ECS_src/BoolExprTreeManager.cpp
        ECS_src/BoolExprTreeManager.h
        ECS_src/ComponentArray.cpp
        ECS_src/ComponentArray.h
        ECS_src/ComponentSparseSet.cpp
        ECS_src/ComponentSparseSet.h
        ECS_src/CoreComponentManagerImpl.cpp
        ECS_src/CoreComponentManagerImpl.h
        ECS_src/UTILS_ECS.h
        ECS_src/ECS_Manager_Port.cpp
        ECS_src/ECS_Manager_Port.h
        ECS_src/ESC_Manager_SystemTraits.h
        ECS_src/SystemImpl.cpp
        ECS_src/SystemImpl.h)
