// WARNING: DUE TO TEMPLATE PARAMETERS, THIS IS NOT A TRANSLATION FILE, IT IS AN IMPLEMENTATION INCLUDED DIRECTLY INTO THE HEADER, ComponentSparseSet.h


#include "ComponentSparseSet.h"






/** Hashed Sparse Set */


template<typename T>
void HASHED_SparseSet::AccommodateAdd(size_t add) {
    while (m_size + add > m_components.size()) {
        m_components.resize(m_components.size() * 2);
        m_handlers.resize(m_handlers.size() * 2);
    }
    m_size += add;
}

template<typename T>
size_t HASHED_SparseSet::DeleteSingle(size_t delPos, size_t shiftedPos) {
    m_components[delPos] = m_components[shiftedPos];
    size_t entInd = m_handlers[delPos] = m_handlers[shiftedPos];
    m_indexMap[entInd] = delPos;
    return entInd;
}

template<typename T>
size_t HASHED_SparseSet::AddAtIndex(size_t ind, size_t newHandle, const T &component) {
    m_indexMap[newHandle] = ind;
    m_handlers[ind] = newHandle;
    m_components[ind] = component;
    return ind;
}


template<typename T>
size_t HASHED_SparseSet::ShiftRegionTo(size_t start, size_t end, size_t dst) {
    std::copy(m_components.begin() + start, m_components.begin() + end, m_components.begin() + dst);
    std::copy(m_handlers.begin() + start, m_handlers.begin() + end, m_handlers.begin() + dst);

    size_t dist = end - start;
    for (size_t e = 0; e < dist; ++e) {
        size_t entInd = m_handlers[dst + e];
        m_indexMap[entInd] = dst + e;
    }
    return dst + (end - start);
}











/** ARRAYED Sparse Set */


template<typename T>
size_t ARRAY_SparseSet::AddAtIndex(size_t ind, size_t newHandle, const T &component) {
    m_indexMap[newHandle] = ind;
    m_handlers[ind] = newHandle;
    m_components[ind] = component;
    return ind;
}

template<typename T>
size_t ARRAY_SparseSet::DeleteSingle(size_t delPos, size_t shiftedPos) {
    m_components[delPos] = m_components[shiftedPos];
    size_t entInd = m_handlers[delPos] = m_handlers[shiftedPos];
    m_indexMap[entInd] = delPos;
    return entInd;
}

template<typename T>
size_t ARRAY_SparseSet::ShiftRegionTo(size_t start, size_t end, size_t dst) {
    std::copy(m_components.begin() + start, m_components.begin() + end, m_components.begin() + dst);
    std::copy(m_handlers.begin() + start, m_handlers.begin() + end, m_handlers.begin() + dst);

    size_t dist = end - start;
    for (size_t e = 0; e < dist; ++e) {
        size_t entInd = m_handlers[dst + e];
        m_indexMap[entInd] = dst + e;
    }
    return dst + (end - start);
}

template<typename T>
void ARRAY_SparseSet::AccommodateAdd(size_t add) {
    while (m_size + add > m_components.size()) {
        m_components.resize(m_components.size() * 2);
        m_handlers.resize(m_handlers.size() * 2);
    }
    m_size += add;
}

template<typename T>
void ARRAY_SparseSet::SetEntitySize(size_t numEnt) {
    size_t s = m_indexMap.size();
    while (s < numEnt) s *= 2;
    m_indexMap.resize(s);
}
