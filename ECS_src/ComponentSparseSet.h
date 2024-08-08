#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <vector>
#include <unordered_map>

using namespace std;

template <typename T, bool UseHashMap>
class SparseSet {};

#define IS_HASHED true
#define IS_ARRAYED false
#define HASHED_SparseSet SparseSet<T, IS_HASHED>
#define ARRAY_SparseSet SparseSet<T, IS_ARRAYED>


template <typename T>
class SparseSet<T, IS_HASHED> {
public:
    unordered_map<size_t, size_t> m_indexMap;
    vector<T> m_components;
    vector<size_t> m_handlers;
    size_t m_size;

    SparseSet() : m_size(64), m_components(m_size), m_handlers(m_size) { }

    void SetEntitySize(size_t numEnt) { }

    T* Get(size_t handler) {  return &m_components[m_indexMap[handler]]; }

    size_t AddAtIndex(size_t ind, size_t newHandle, const T& component);

    /*
    * 'Deletes' an element by overwriting it with the element at shiftedPos,
    * UPDATES SHIFT'S HANDLE BUT NOT DEL'S
    */
    size_t DeleteSingle(size_t delPos, size_t shiftedPos);

    /*
    * WARNING ASSUMES THE REGION IS EMPTY, BASICALLY CONSIDERS IT A DELETE
    *
    * Equivalent to std::copy but also updates the m_indexMap.
    */
    size_t ShiftRegionTo(size_t start, size_t end, size_t dst);

    void AccommodateAdd(size_t add);
};


template <typename T>
class SparseSet<T, IS_ARRAYED> {
public:
    vector<size_t> m_indexMap;
    vector<T> m_components;
    vector<size_t> m_handlers;
    size_t m_size;

    SparseSet() {
        m_size = 64;
        m_components.resize(m_size);
        m_handlers.resize(m_size);
        m_indexMap.resize(m_size);
    }

    void SetEntitySize(size_t numEnt);

    T* Get(size_t handler) { return &m_components[m_indexMap[handler]]; }

    size_t AddAtIndex(size_t ind, size_t newHandle, const T& component);

    /*
    * 'Deletes' an element by overwriting it with the element at shiftedPos,
    * UPDATES SHIFT'S HANDLE BUT NOT DEL'S
    */
    size_t DeleteSingle(size_t delPos, size_t shiftedPos);

    /*
    * WARNING ASSUMES THE REGION IS EMPTY, BASICALLY CONSIDERS IT A DELETE
    *
    * Equivalent to std::copy but also updates the m_indexMap.
    */
    size_t ShiftRegionTo(size_t start, size_t end, size_t dst);

    void AccommodateAdd(size_t add);
};

#include "ComponentSparseSet.cpp"


#endif