#pragma once

#include <vector>
#include <string>
#include <QString>
#include <QDir>

#include "ui/Blackhole.h"

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

QString absolutePath(const QString& gamePath);

bool fileExists(const QString& file);
