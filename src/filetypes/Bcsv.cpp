#include "filetypes/Bcsv.h"


Bcsv::EntryVal& Bcsv::Entry::operator[](QString& key)
{
    return m_entry[Bcsv::fieldNameToHash(key)];
}

Bcsv::EntryVal& Bcsv::Entry::get(QString& key, Bcsv::EntryVal& defaultValue)
{
    uint32_t hash = fieldNameToHash(key);
    auto loc = m_entry.find(hash);

    if(loc != m_entry.end())
        return loc->second;

    return defaultValue;
}

uint32_t Bcsv::fieldNameToHash(QString& fieldName)
{
    uint32_t ret = 0;
    for(char c : fieldName.toStdString())
    {
        ret *= 0x1F;
        ret += c;
    }

    return ret;
}

QString Bcsv::hashToFieldName(uint32_t hash)
{
    auto loc = LOOKUP.find(hash);
    if(loc == LOOKUP.end())
        return QString("[%1$08X]").arg(hash);

    return loc->second;
}

void Bcsv::addHash(QString field)
{
    uint32_t hash = fieldNameToHash(field);
    if(LOOKUP.find(hash) == LOOKUP.end())
        LOOKUP.insert(std::make_pair(hash, field));
}

std::unordered_map<uint32_t, QString> Bcsv::LOOKUP;
