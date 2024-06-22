#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <vector>
#include <unordered_map>

using namespace std;

template <typename T, bool UseHashMap>
class SparseSet {};

#define HASHED_SparseSet SparseSet<T, true>
#define ARRAY_SparseSet SparseSet<T, false>

/**
 * HASHED_SparseSet
 * @tparam T
 */
template <typename T>
class SparseSet<T, true> {
public:
    unordered_map<size_t, size_t> _indexMap;
    vector<T> _components;
    vector<size_t> _handlers;
    size_t _size;

    SparseSet() : _size(64), _components(_size), _handlers(_size) { }

    void SetEntitySize(size_t numEnt) { }

    T* Get(size_t handler) {  return &_components[_indexMap[handler]]; }

    size_t AddAtIndex(size_t ind, size_t newHandle, const T& component);

    /*
    * 'Deletes' an element by overwriting it with the element at shiftedPos,
    * UPDATES SHIFTED'S HANDLE BUT NOT DEL'S
    */
    size_t DeleteSingle(size_t delPos, size_t shiftedPos);

    /*
    * WARNING ASSUMES THE REGION IS EMPTY, BASCIALLY CONSIDERS IT A DELETE
    *
    * Equivalent to std::copy but also updates the _indexMap.
    */
    size_t ShiftRegionTo(size_t start, size_t end, size_t dst);

    void AccomodateAdd(size_t add);
};

/**
 * ARRAY_SparseSet
 * @tparam T
 */
template <typename T>
class SparseSet<T, false> {
public:
    vector<size_t> _indexMap;
    vector<T> _components;
    vector<size_t> _handlers;
    size_t _size;

    SparseSet() {
        _size = 64;
        _components.resize(_size);
        _handlers.resize(_size);
        _indexMap.resize(_size);
    }

    void SetEntitySize(size_t numEnt);

    T* Get(size_t handler) { return &_components[_indexMap[handler]]; }

    size_t AddAtIndex(size_t ind, size_t newHandle, const T& component);

    /*
    * 'Deletes' an element by overwriting it with the element at shiftedPos,
    * UPDATES SHIFTED'S HANDLE BUT NOT DEL'S
    */
    size_t DeleteSingle(size_t delPos, size_t shiftedPos);

    /*
    * WARNING ASSUMES THE REGION IS EMPTY, BASCIALLY CONSIDERS IT A DELETE
    *
    * Equivalent to std::copy but also updates the _indexMap.
    */
    size_t ShiftRegionTo(size_t start, size_t end, size_t dst);

    void AccomodateAdd(size_t add);
};

#include "ComponentSparseSet.cpp"


#endif