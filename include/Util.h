#pragma once

#include <span>
#include <vector>
#include <string>
#include <QString>
#include <QDir>

#include <glm/glm.hpp>

#include "ui/Blackhole.h"

#define BLACKHOLE_ENUM_START(name) namespace name { enum name : uint8_t
#define BLACKHOLE_ENUM_END(name) \
}; typedef name::name name##_t;

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

namespace std {

    template <typename T, size_t Extent>
    bool operator==(span<T, Extent> lhs, span<T> rhs)
    {
        return std::equal(begin(lhs), end(lhs), begin(rhs), end(rhs));
    }

} // namespace std

QString absolutePath(const QString& gamePath);

bool fileExists(const QString& file);

float lerp(float a, float b, float t);
glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t);
