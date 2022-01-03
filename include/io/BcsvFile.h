#pragma once

#include "io/FileBase.h"

#include <unordered_map>
#include <vector>
#include <variant>
#include <QString>


class BcsvFile
{
public:
    static uint32_t fieldNameToHash(const QString& fieldName);
    static QString hashToFieldName(uint32_t hash);
    static void addHash(QString field);

    struct Field {
        uint32_t nameHash;
        uint32_t mask; // wear it
        uint16_t entryOffset;
        uint8_t shift;
        uint8_t type;

        QString name;

        constexpr static int dataSizes[] = { 4, -1, 4, 4, 2, 1, 4 };
    };

    typedef std::variant<uint32_t, uint16_t, uint8_t, float, QString> Value;

    class Entry
    {
        mutable std::unordered_map<uint32_t, Value> m_entry;

    public:
        Value operator[](const QString& key) const;
        Value operator[](uint32_t key) const;

        Value get(const QString& key, const Value& defaultValue) const;

        void insert(const QString& key, const Value& val);
        void insert(uint32_t key, const Value& val);

        void erase(const QString& key);
        void erase(uint32_t key);

        bool contains(const QString& key) const;
    };

private:
    static std::unordered_map<uint32_t, QString> LOOKUP;


    // ------------------------
    // actual class starts here
    // ------------------------

    FileBase* file;

public:
    BcsvFile(FileBase* inRarcFile);

    void save();
    void close();

    Field addField(const QString& name, uint32_t mask, uint16_t offset, uint8_t shift, uint8_t type,  Value defaultValue);

    void removeField(const QString& name);

    std::unordered_map<uint32_t, Field> fields;
    std::vector<Entry> entries;
};
