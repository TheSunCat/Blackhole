#pragma once

#include "io/BaseFile.h"

#include <unordered_map>
#include <vector>
#include <variant>
#include <QString>


class BcsvFile
{
public:
    // ----------------------------
    // using class as namespace lol
    // ----------------------------

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

    struct _ValueProxy
    {
        Value& var;

        operator uint32_t()&&;
        operator uint16_t()&&;
        operator uint8_t()&&;
        operator float()&&;
        operator QString()&&;

        template<typename T>
        _ValueProxy operator=(T& other)
        {
            var = other;
        }
    };

    class Entry
    {
        mutable std::unordered_map<uint32_t, Value> m_entry;

    public:

        _ValueProxy operator[](const QString& key) const;
        _ValueProxy operator[](uint32_t key) const;

        _ValueProxy get(const QString& key, Value defaultValue) const;

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

    BaseFile* file;

public:
    BcsvFile(BaseFile* inRarcFile);

    void save();
    void close();

    Field addField(const QString& name, uint32_t mask, uint16_t offset, uint8_t shift, uint8_t type,  Value defaultValue);

    void removeField(const QString& name);

    std::unordered_map<uint32_t, Field> fields;
    std::vector<Entry> entries;
};
