#pragma once

#include "io/BaseFile.h"

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

    class Entry
    {
        mutable std::unordered_map<uint32_t, Value> m_entry;

    public:

        Value operator[](const QString& key) const;
        Value operator[](uint32_t key) const;

        Value get(const QString& key, Value defaultValue) const;
        Value get(uint32_t key, Value defaultValue) const;
        uint32_t geti(const QString& key, uint32_t defaultValue = 0) const;
        uint32_t geti(uint32_t key, uint32_t defaultValue = 0) const;
        uint16_t gets(const QString& key, uint16_t defaultValue = 0) const;
        uint16_t gets(uint32_t key, uint16_t defaultValue = 0) const;
        uint8_t getb(const QString& key, uint8_t defaultValue = 0) const;
        uint8_t getb(uint32_t key, uint8_t defaultValue = 0) const;
        float getf(const QString& key, float defaultValue = 0) const;
        float getf(uint32_t key, float defaultValue = 0) const;
        QString getstr(const QString& key, const QString& defaultValue = "") const;
        QString getstr(uint32_t key, const QString& defaultValue = "") const;

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

    std::vector<Field> m_fields;
    std::vector<Entry> m_entries;
};
