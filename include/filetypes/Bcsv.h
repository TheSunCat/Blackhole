#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <variant>
#include <QString>


class Bcsv
{
public:
    static uint32_t fieldNameToHash(QString& fieldName);
    static QString hashToFieldName(uint32_t hash);
    static void addHash(QString field);

private:
    static std::unordered_map<uint32_t, QString> LOOKUP;

    struct Field {
        uint32_t nameHash;
        uint32_t mask;
        uint16_t entryOffset;
        uint8_t shiftAmount;
        uint8_t type;

        QString name;
    };

    typedef std::variant<QString, uint32_t, float> EntryVal;

    class Entry
    {
        std::unordered_map<uint32_t, EntryVal> m_entry;

    public:
        EntryVal& operator[](QString& key);
        EntryVal& get(QString& key, EntryVal& defaultValue);

        bool contains(QString& key);
    };

    std::unordered_map<uint32_t, Bcsv::Field> fields;
    std::vector<Bcsv::Entry> entries;

};
