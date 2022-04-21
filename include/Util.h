#pragma once

#include <span>
#include <vector>
#include <string>
#include <QString>
#include <QDir>

#include <glm/glm.hpp>

#include "ui/Blackhole.h"

// workaround for bad enums in C++
#define BLACKHOLE_ENUM_START(name) namespace name { enum name : uint8_t
#define BLACKHOLE_ENUM_END(name) \
}; typedef name::name name##_t;

// sometimes M_PI is not defined
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

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


/*template<typename T>
class VectorView
{
    typedef typename  Iterator; // TODO std::vector<T>::const_iterator

    const T* m_begin = nullptr;
    const T* m_end = nullptr;
public:
    VectorView(const T* begin, const T* end) : m_begin(begin), m_end(end) {}
    const T* begin() const { return m_begin; }
    const T* end()   const { return m_end;   }

    typename std::iterator_traits<const T*>::reference operator[](std::size_t index) { return m_begin[index]; };
};*/

template<typename T, std::size_t H>
uint32_t indexOf(std::array<T, H> array, T element)
{
    for(uint32_t i = 0; i < array.size(); i++)
    {
        if(array[i] == element)
            return i;
    }

    return -1;
}

namespace std {

    template <typename T, size_t Extent>
    bool operator==(span<T, Extent> lhs, span<T> rhs)
    {
        return std::equal(begin(lhs), end(lhs), begin(rhs), end(rhs));
    }

} // namespace std

namespace Util
{
    QString absolutePath(const QString& gamePath);

    bool fileExists(const QString& file);

    float lerp(float a, float b, float t);
    glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t);
};
