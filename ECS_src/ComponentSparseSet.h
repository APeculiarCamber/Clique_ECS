#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <vector>
#include <unordered_map>

using namespace std;

template <typename T, bool UseHashMap>
class SparseSet {};

#define HASHED_SparseSet SparseSet<T, true>
#define ARRAY_SparseSet SparseSet<T, false>

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


/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/



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







/** Hashed Sparse Set */


template<typename T>
void HASHED_SparseSet::AccomodateAdd(size_t add) {
    while (_size + add > _components.size()) {
        _components.resize(_components.size() * 2);
        _handlers.resize(_handlers.size() * 2);
    }
    // TODO : size depends on controlling classes!!!
    _size += add;
}

template<typename T>
size_t HASHED_SparseSet::DeleteSingle(size_t delPos, size_t shiftedPos) {
    _components[delPos] = _components[shiftedPos];
    size_t entInd = _handlers[delPos] = _handlers[shiftedPos];
    _indexMap[entInd] = delPos;
    return entInd;
}

template<typename T>
size_t HASHED_SparseSet::AddAtIndex(size_t ind, size_t newHandle, const T &component) {
    _indexMap[newHandle] = ind;
    _handlers[ind] = newHandle;
    _components[ind] = component;
    return ind;
}


template<typename T>
size_t HASHED_SparseSet::ShiftRegionTo(size_t start, size_t end, size_t dst) {
    std::copy(_components.begin() + start, _components.begin() + end, _components.begin() + dst);
    std::copy(_handlers.begin() + start, _handlers.begin() + end, _handlers.begin() + dst);

    size_t dist = end - start;
    for (size_t e = 0; e < dist; ++e) {
        size_t entInd = _handlers[dst + e];
        _indexMap[entInd] = dst + e;
    }
    return dst + (end - start);
}











/** ARRAYED Sparse Set */


template<typename T>
size_t ARRAY_SparseSet::AddAtIndex(size_t ind, size_t newHandle, const T &component) {
    _indexMap[newHandle] = ind;
    _handlers[ind] = newHandle;
    _components[ind] = component;
    return ind;
}

template<typename T>
size_t ARRAY_SparseSet::DeleteSingle(size_t delPos, size_t shiftedPos) {
    _components[delPos] = _components[shiftedPos];
    size_t entInd = _handlers[delPos] = _handlers[shiftedPos];
    _indexMap[entInd] = delPos;
    return entInd;
}

template<typename T>
size_t ARRAY_SparseSet::ShiftRegionTo(size_t start, size_t end, size_t dst) {
    std::copy(_components.begin() + start, _components.begin() + end, _components.begin() + dst);
    std::copy(_handlers.begin() + start, _handlers.begin() + end, _handlers.begin() + dst);

    size_t dist = end - start;
    for (size_t e = 0; e < dist; ++e) {
        size_t entInd = _handlers[dst + e];
        _indexMap[entInd] = dst + e;
    }
    return dst + (end - start);
}

template<typename T>
void ARRAY_SparseSet::AccomodateAdd(size_t add) {
    while (_size + add > _components.size()) {
        _components.resize(_components.size() * 2);
        _handlers.resize(_handlers.size() * 2);
    }
    // TODO : size depends on controlling classes!!!
    _size += add;
}

template<typename T>
void ARRAY_SparseSet::SetEntitySize(size_t numEnt) {
    size_t s = _indexMap.size();
    while (s < numEnt) s *= 2;
    _indexMap.resize(s);
}


#endif