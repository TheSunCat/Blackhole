#include "io/BmdFile.h"

#include <stack>

BmdFile::BmdFile(BaseFile* inRarcFile) : file(inRarcFile)
{
    file->setBigEndian(true);

    file->position(0xC);
    uint32_t numSections = file->readInt();

    file->skip(0x10);

    for(uint32_t i = 0; i < numSections; i++)
    {
        QString section= file->readString(4, "ASCII");
        if(section == "INF1")
            readINF1();
        if(section == "VTX1")
            readVTX1();
        if(section == "EVP1")
            readEVP1();
        if(section == "DRW1")
            readDRW1();
        // TODO other sections
    }
}

void BmdFile::readINF1()
{
    uint32_t sectionStart = file->position() - 0x4;
    uint32_t sectionSize = file->readInt();

    m_sceneGraph.clear();

    std::stack<uint16_t> materialStack;
    std::stack<uint16_t> nodeStack;

    materialStack.push(0xFFFF);
    nodeStack.push(-1); // TODO -1?

    file->skip(8);
    m_vertexCount = file->readInt();

    uint32_t dataStart = file->readInt();
    file->skip(dataStart - 0x18);

    uint16_t curType = 0;
    while((curType = file->readShort()) != 0)
    {
        uint16_t arg = file->readShort();

        switch(curType)
        {
            case 0x01:
            {
                materialStack.push(materialStack.top());
                nodeStack.push(m_sceneGraph.size() - 1);
                break;
            }
            case 0x02:
            {
                materialStack.pop();
                nodeStack.pop();
                break;
            }
            case 0x11:
            {
                materialStack.pop();
                nodeStack.push(arg);
                break;
            }
            case 0x10:
            case 0x12:
            {
                SceneGraphNode newNode = SceneGraphNode{
                    materialStack.top(),
                    nodeStack.top(),
                    uint32_t((curType == 0x12) ? 0 : 1),
                    arg
                };

                m_sceneGraph.push_back(newNode);
                break;
            }
        }
    }

    file->position(sectionStart + sectionSize);
}

void BmdFile::readVTX1()
{
    uint32_t sectionStart = file->position() - 0x4;
    uint32_t sectionSize = file->readInt();

    m_arrayMask = 0;

    m_colors[0].clear(); m_colors[1].clear();
    for(int i = 0; i < m_texCoords.size(); i++)
        m_texCoords[i].clear();

    std::vector<uint32_t> arrayOffsets;

    uint32_t arrayDefOffset = file->readInt();
    for(int i = 0; i < 13; i++)
    {
        file->position(sectionStart + 0xC + (i * 0x4));
        uint32_t dataOffset = file->readInt();

        if(dataOffset == 0)
            continue;

        arrayOffsets.push_back(dataOffset);
    }

    for(int i = 0; i < arrayOffsets.size(); i++)
    {
        file->position(sectionStart + arrayDefOffset + (i * 0x10));

        uint32_t arrayType = file->readInt();
        uint32_t compSize = file->readInt();
        uint32_t dataType = file->readInt();
        uint8_t fixedPoint = file->readByte() & 0xFF; // TODO do we need this & 0xFF?

        // Whitehole comment:
        // apparently, arrays may contain more elements than specified in the INF1 section
        // so we have to rely on bmdview2's way to know the array's exact size
        uint32_t arraySize = 0;
        if(i == arrayOffsets.size() - 1)
            arraySize = sectionSize - arrayOffsets[i];
        else
            arraySize = arrayOffsets[i + 1] - arrayOffsets[i];

        if(arrayType == 11 || arrayType == 12)
        {
            assert(!((dataType < 3) ^ (compSize == 0))); // Bmd: component count mismatch in color array

            switch(dataType)
            {
                case 1:
                case 2:
                case 5:
                    arraySize /= 4;
                    break;
                default:
                    assert(false); // Bmd: unsupported color DataType
            }
        }
        else
        {
            switch(dataType)
            {
                case 3:
                {
                    arraySize /= 2;
                    break;
                }
                case 4:
                {
                    arraySize /= 4;
                    break;
                }
                default:
                    assert(false); // Bmd: unsupported DataType
            }
        }

        file->position(sectionStart + arrayOffsets[i]);

        m_arrayMask |= 0b1 << arrayType;
        switch(arrayType)
        {
            case 9:
            {
                if(compSize == 0)
                {
                    m_positions.reserve(arraySize / 2);
                    for(int j = 0; j < arraySize / 2; j++)
                        m_positions[j] = glm::vec3(readArrayValue(dataType, fixedPoint),
                                                   readArrayValue(dataType, fixedPoint), 0);
                }
                else if(compSize == 1)
                {
                    m_positions.reserve(arraySize / 3);
                    for(int j = 0; j < arraySize / 3; j++)
                        m_positions[j] = glm::vec3(readArrayValue(dataType, fixedPoint),
                                                   readArrayValue(dataType, fixedPoint),
                                                   readArrayValue(dataType, fixedPoint));

                }
                else
                {
                    assert(false); // Bmd: unsupported position CompSize
                }

                break;
            }
            case 10:
            {
                if(compSize == 0)
                {
                    m_normals.reserve(arraySize / 3);
                    for(int j = 0; j < arraySize / 3; j++)
                        m_normals[j] = glm::vec3(readArrayValue(dataType, fixedPoint),
                                                 readArrayValue(dataType, fixedPoint),
                                                 readArrayValue(dataType, fixedPoint));
                }
                else
                {
                    assert(false); // Bmd: unsupported normal CompSize
                }

                break;
            }
            case 11:
            case 12:
            {
                uint32_t cid = arrayType - 11;
                m_colors[cid] = std::vector<QColor>(arraySize);
                for(int j = 0; j < arraySize; j++)
                    m_colors[cid][j] = readColorValue(dataType);

                break;
            }
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            {
                uint32_t tid = arrayType - 13;
                if(compSize == 0)
                {
                    m_texCoords[tid].reserve(arraySize);
                    for(int j = 0; j < arraySize; j++)
                        m_texCoords[tid][j] = glm::vec2(readArrayValue(dataType, fixedPoint), 0);
                }
                else if(compSize == 1)
                {
                    m_texCoords[tid].reserve(arraySize / 2);
                    for(int j = 0; j < arraySize / 2; j++)
                        m_texCoords[tid][j] = glm::vec2(readArrayValue(dataType, fixedPoint),
                                                        readArrayValue(dataType, fixedPoint));
                }
                else
                {
                    assert(false); // Bmd: unsupported texcoord CompSize
                }
                break;
            }
        }
    }

    file->position(sectionStart + sectionSize);
}

void BmdFile::readEVP1()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    uint16_t count = file->readShort();
    file->skip(0x2);

    m_multiMatrices.clear();
    m_multiMatrices.reserve(count);

    uint32_t sizesOffset = file->readInt();
    uint32_t indicesOffset = file->readInt();
    uint32_t weightsOffset = file->readInt();
    uint32_t matricesOffset = file->readInt();

    uint32_t position1 = 0, position2 = 0;
    for(uint32_t i = 0; i < count; i++)
    {
        file->position(sectionStart + sizesOffset + i);

        uint8_t subCount = file->readByte();

        MultiMatrix multiMatrix{
            subCount
        };
        multiMatrix.indices.reserve(subCount);
        multiMatrix.matrices.reserve(subCount);
        multiMatrix.weights.reserve(subCount);

        for(uint32_t j = 0; j < subCount; j++)
        {
            file->position(sectionStart + indicesOffset + position1);
            multiMatrix.indices[j] = file->readShort();
            position1 += 2;

            file->position(sectionStart + weightsOffset + position2);
            multiMatrix.weights[j] = file->readFloat();
            position2 += 4;

            file->position(sectionStart + matricesOffset + (multiMatrix.indices[j] * 0x30));

            // GLM is column-major
            multiMatrix.matrices[j][0][0] = file->readFloat();
            multiMatrix.matrices[j][0][1] = file->readFloat();
            multiMatrix.matrices[j][0][2] = file->readFloat();
            multiMatrix.matrices[j][0][3] = file->readFloat();
            multiMatrix.matrices[j][1][0] = file->readFloat();
            multiMatrix.matrices[j][1][1] = file->readFloat();
            multiMatrix.matrices[j][1][2] = file->readFloat();
            multiMatrix.matrices[j][1][3] = file->readFloat();
            multiMatrix.matrices[j][2][0] = file->readFloat();
            multiMatrix.matrices[j][2][1] = file->readFloat();
            multiMatrix.matrices[j][2][2] = file->readFloat();
            multiMatrix.matrices[j][2][3] = file->readFloat();
            multiMatrix.matrices[j][3][0] = file->readFloat();
            multiMatrix.matrices[j][3][1] = file->readFloat();
            multiMatrix.matrices[j][3][2] = file->readFloat();
            multiMatrix.matrices[j][3][3] = file->readFloat();
        }
    }

    file->position(sectionStart + sectionSize);
}

void BmdFile::readDRW1()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    uint16_t count = file->readShort();
    file->skip(0x2);

    m_matrixTypes.clear();
    m_matrixTypes.reserve(count);

    uint32_t weightedsOffset = file->readInt();
    uint32_t indicesOffset = file->readInt();

    for(uint32_t i = 0; i < count; i++)
    {
        file->position(sectionStart + weightedsOffset + i);
        m_matrixTypes[i].weighted = (file->readByte() != 0);

        file->position(sectionStart + indicesOffset + (i * 2));
        m_matrixTypes[i].index = file->readShort();
    }

    file->position(sectionStart + sectionSize);
}


float BmdFile::readArrayShort(uint8_t fixedPoint)
{
    short val = file->readShort();
    return (float)(val / (float)(1 << fixedPoint));
}

float BmdFile::readArrayFloat()
{
    return file->readFloat();
}

float BmdFile::readArrayValue(uint32_t type, uint8_t fixedPoint)
{
    if(type == 3)
        return readArrayShort(fixedPoint);
    if(type == 4)
        return readArrayFloat();

    return 0;
}

QColor BmdFile::readColor_RGBA8()
{
    int r = file->readByte() & 0xFF;
    int g = file->readByte() & 0xFF;
    int b = file->readByte() & 0xFF;
    int a = file->readByte() & 0xFF;
    return QColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
}

QColor BmdFile::readColor_RGBX8()
{
    int r = file->readByte() & 0xFF;
    int g = file->readByte() & 0xFF;
    int b = file->readByte() & 0xFF;
    file->readByte();
    return QColor(r / 255.f, g / 255.f, b / 255.f, 1.f);
}

QColor BmdFile::readColorValue(uint32_t type)
{
    switch (type)
    {
        case 1:
        case 2:
            return readColor_RGBX8();
        case 5:
            return readColor_RGBA8();
    }

    return QColor();
}
