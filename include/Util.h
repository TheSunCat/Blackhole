#pragma once

#include <vector>

// TODO this looks horrid. is there a better way?
template<class T>
class Iterator
{
    std::vector<T>* m_vec;
    typename std::vector<T>::iterator m_it;

public:
    Iterator(std::vector<T>* vector) : m_vec(vector), m_it(m_vec->begin()) {}

    typename std::vector<T>::iterator operator++()
    {
        return ++m_it;
    }

    bool hasNext()
    {
        return (m_it + 1) != m_vec->end();
    }

    T operator*()
    {
        return *m_it;
    }
};
