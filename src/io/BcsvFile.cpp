#include "io/BcsvFile.h"

#include <QHash>

BcsvFile::BcsvFile(BaseFile* inRarcFile) : file(inRarcFile)
{
    uint32_t entryCount = file->readInt();
    uint32_t fieldCount = file->readInt();
    uint32_t dataOffset = file->readInt();
    uint32_t entryDataSize = file->readInt();

    uint32_t stringTableOffset = dataOffset + (entryCount * entryDataSize);

    for(int i = 0; i < fieldCount; i++)
    {
        file->position(0x10 + 0xC * i);

        Field field {
            file->readInt(),   // nameHash
            file->readInt(),   // mask
            file->readShort(), // entryOffset
            file->readByte(),  // shiftAmount
            file->readByte(),  // type
            BcsvFile::hashToFieldName(field.nameHash)
        };

        m_fields.push_back(field);
    }

    for(int i = 0; i < entryCount; i++)
    {
        Entry entry;

        for(const Field& field : m_fields)
        {
            file->position(dataOffset + (i * entryDataSize) + field.entryOffset);

            Value val;
            switch(field.type) {
                case 0:
                case 3:
                {
                    val = uint32_t((file->readInt() & field.mask) >> field.shift);
                    break;
                }
                case 4:
                {
                    val = uint16_t((file->readShort() & field.mask) >> field.shift);
                    break;
                }
                case 5:
                {
                    val = uint8_t((file->readByte() * field.mask) >> field.shift);
                    break;
                }
                case 2:
                {
                    val = file->readFloat();
                    break;
                }
                case 6:
                {
                    int strOffset = file->readInt();
                    file->position(stringTableOffset + strOffset);
                    val = file->readString(0, "Shift_JIS");
                    break;
                }
                default:
                {
                    assert(false); // Bcsv: unsupported data type
                }
            }

            entry.insert(field.nameHash, val);
        }

        m_entries.emplace_back(entry);
    }
}

void BcsvFile::save()
{
    uint32_t entrySize = 0;

    // grow entrySize to the largest field's size
    for(const Field& field : m_fields)
    {
        uint16_t fieldEnd = field.entryOffset + Field::dataSizes[field.type];

        if(fieldEnd > entrySize)
            entrySize = fieldEnd;
    }

    entrySize = (entrySize + 3) & ~3; // TODO simplify this in bin. why is it written this way?

    uint32_t dataOffset = 0x10 + (0xC * m_fields.size());
    uint32_t stringTableOffset = dataOffset + (entrySize * m_entries.size());

    uint32_t curString = 0;

    file->setLength(stringTableOffset);
    file->writeInt(m_entries.size());
    file->writeInt(m_fields.size());
    file->writeInt(dataOffset);
    file->writeInt(entrySize);

    for(const Field& field : m_fields)
    {
        file->writeInt(field.nameHash);
        file->writeInt(field.mask);
        file->writeShort(field.entryOffset);
        file->writeByte(field.shift);
        file->writeByte(field.type);
    }

    int i = 0;
    std::unordered_map<QString, int> stringOffsets;

    for(const Entry& entry : m_entries)
    {
        for(const Field& field : m_fields)
        {
            uint32_t valOffset = dataOffset + (i * entrySize) + field.entryOffset;
            file->position(valOffset);

            switch(field.type)
            {
            case 0:
            case 3:
            {
                uint32_t val = file->readInt();
                val &= ~field.mask;
                val |= (std::get<uint32_t>(entry[field.nameHash]) << field.shift) & field.mask;

                file->position(valOffset);
                file->writeInt(val);
                break;
            }
            case 4:
            {
                uint16_t val = file->readShort();
                val &= ~field.mask;
                val |= (std::get<uint16_t>(entry[field.nameHash]) << field.shift) & field.mask;

                file->position(valOffset);
                file->writeShort(val);
                break;
            }
            case 5:
            {
                uint8_t val = file->readShort();
                val &= ~field.mask;
                val |= (entry.gets(field.nameHash) << field.shift) & field.mask;

                file->position(valOffset);
                file->writeByte(val);
                break;
            }
            case 2:
            {
                file->writeFloat(entry.getf(field.nameHash));
                break;
            }
            case 6:
            {
                QString val = std::get<QString>(entry[field.nameHash]);
                if(stringOffsets.find(val) != stringOffsets.end())
                {
                    file->writeInt(stringOffsets[val]);
                }
                else
                {
                    stringOffsets[val] = curString;
                    file->writeInt(curString);
                    file->position(stringTableOffset + curString);
                    curString += file->writeString(val, "Shift_JIS");
                }

                break;
            }
            }

        }

        i++;
    }

    i = file->getLength();
    file->position(i);
    int alignedEnd = (i + 0x1F) & ~0x1F;
    for(; i < alignedEnd; i++)
        file->writeByte(0x40);

    file->save();
}

void BcsvFile::close()
{
    file->close();
}

BcsvFile::Field BcsvFile::addField(const QString& name, uint32_t mask, uint16_t offset, uint8_t shift, uint8_t type, Value defaultValue)
{
    addHash(name); // nyeh heh heh

    if(type == 2 || type == 6)
    {
        mask = 0xFFFFFFFF;
        shift = 0;
    }

    if(offset == MAX_U16)
    {
        for(const Field& field : m_fields)
        {
            uint16_t fieldEnd = field.entryOffset + Field::dataSizes[field.type];

            if(fieldEnd > offset)
                offset = fieldEnd;
        }
    }

    Field newField {
        fieldNameToHash(name),
        mask,
        offset,
        shift,
        type,
        name
    };

    m_fields.push_back(newField);

    for(Entry& entry : m_entries)
        entry[name] = defaultValue;

    return newField;
}

void BcsvFile::removeField(const QString& name)
{
    uint32_t hash = fieldNameToHash(name);

    // delete the field with this hash
    std::erase_if(m_fields, [&](const Field& f) { return f.nameHash == hash;});

    for(Entry& entry : m_entries)
        entry.erase(hash);
}



BcsvFile::Value BcsvFile::Entry::operator[](const QString& key) const
{
    return operator[](BcsvFile::fieldNameToHash(key));
}

BcsvFile::Value BcsvFile::Entry::operator[](uint32_t key) const
{
    return m_entry.at(key);
}

bool BcsvFile::Entry::contains(const QString& key) const
{
    uint32_t hash = fieldNameToHash(key);
    auto loc = m_entry.find(hash);

    return loc != m_entry.end();
}

BcsvFile::Value BcsvFile::Entry::get(const QString& key, BcsvFile::Value defaultValue) const
{
    return get(fieldNameToHash(key), defaultValue);
}

BcsvFile::Value BcsvFile::Entry::get(uint32_t key, BcsvFile::Value defaultValue) const
{
    auto loc = m_entry.find(key);

    if(loc != m_entry.end())
        return loc->second;

    return defaultValue;
}

uint32_t BcsvFile::Entry::geti(const QString& key, uint32_t defaultValue) const
{
    return std::get<uint32_t>(get(key, Value(defaultValue)));
}

uint32_t BcsvFile::Entry::geti(uint32_t key, uint32_t defaultValue) const
{
    return std::get<uint32_t>(get(key, Value(defaultValue)));
}

uint16_t BcsvFile::Entry::gets(const QString& key, uint16_t defaultValue) const
{
    return std::get<uint16_t>(get(key, Value(defaultValue)));
}

uint16_t BcsvFile::Entry::gets(uint32_t key, uint16_t defaultValue) const
{
    return std::get<uint16_t>(get(key, Value(defaultValue)));
}

uint8_t BcsvFile::Entry::getb(const QString& key, uint8_t defaultValue) const
{
    return std::get<uint8_t>(get(key, Value(defaultValue)));
}

uint8_t BcsvFile::Entry::getb(uint32_t key, uint8_t defaultValue) const
{
    return std::get<uint8_t>(get(key, Value(defaultValue)));
}

float BcsvFile::Entry::getf(const QString& key, float defaultValue) const
{
    return std::get<float>(get(key, Value(defaultValue)));
}

float BcsvFile::Entry::getf(uint32_t key, float defaultValue) const
{
    return std::get<float>(get(key, Value(defaultValue)));
}

QString BcsvFile::Entry::getstr(const QString& key, const QString& defaultValue) const
{
    return std::get<QString>(get(key, Value(defaultValue)));
}

QString BcsvFile::Entry::getstr(uint32_t key, const QString& defaultValue) const
{
    return std::get<QString>(get(key, Value(defaultValue)));
}

void BcsvFile::Entry::insert(const QString& key, const Value& val)
{
    m_entry.insert(std::make_pair(fieldNameToHash(key), val));
}

void BcsvFile::Entry::insert(uint32_t key, const Value& val)
{
    m_entry.insert(std::make_pair(key, val));
}

void BcsvFile::Entry::erase(const QString& key)
{
    uint32_t hash = fieldNameToHash(key);
    erase(hash);
}

void BcsvFile::Entry::erase(uint32_t key)
{
    m_entry.erase(key);
}


uint32_t BcsvFile::fieldNameToHash(const QString& fieldName)
{
    uint32_t ret = 0;
    for(char c : fieldName.toStdString())
    {
        ret *= 0x1F;
        ret += c;
    }

    return ret;
}

QString BcsvFile::hashToFieldName(uint32_t hash)
{
    auto loc = LOOKUP.find(hash);
    if(loc == LOOKUP.end())
        return QString("[%1$08X]").arg(hash);

    return loc->second;
}

void BcsvFile::addHash(QString field)
{
    uint32_t hash = fieldNameToHash(field);
    if(LOOKUP.find(hash) == LOOKUP.end())
        LOOKUP.insert(std::make_pair(hash, field));
}

std::unordered_map<uint32_t, QString> BcsvFile::LOOKUP;

