#include "io/BmdFile.h"

#include <stack>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

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
        if(section == "JNT1")
            readJNT1();
        if(section == "SHP1")
            readSHP1();
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

void BmdFile::readJNT1()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    uint16_t jointCount = file->readShort();
    file->skip(0x2);

    m_joints.clear();
    m_joints.reserve(jointCount);

    uint32_t jointsOffset = file->readInt();
    uint32_t unksOffset = file->readInt();
    uint32_t stringsOffset = file->readInt();

    for(uint32_t i = 0; i < jointCount; i++)
    {
        file->position(sectionStart + jointsOffset + (i * 0x40));

        Joint& joint = m_joints[i];
        joint.unk1 = file->readShort();
        joint.unk2 = file->readByte();
        file->skip(0x1);

        joint.scale = readVec3();
        joint.rotation = glm::vec3(
            float(file->readShort()),
            float(file->readShort()),
            float(file->readShort())
        )  * float(M_PI / 32768.f);

        file->skip(0x2);

        joint.translation = readVec3();

        glm::mat4 mat = glm::scale(glm::mat4(1.0), joint.scale);
        mat = glm::eulerAngleXYZ(joint.rotation.x, joint.rotation.y, joint.rotation.z) * mat;
        mat = glm::translate(mat, joint.translation);

        for(SceneGraphNode& node : m_sceneGraph)
        {
            if(node.nodeID != i || node.nodeType != 1)
                continue;

            SceneGraphNode* parentNode = &node;
            do
            {
                if (parentNode->parentIndex == -1)
                {
                    parentNode = nullptr;
                    break;
                }

                parentNode = &m_sceneGraph[parentNode->parentIndex];
            } while(parentNode->nodeType != 1);

            if(parentNode != nullptr)
                joint.finalMatrix = joint.matrix * m_joints[parentNode->nodeID].finalMatrix;
            else
                joint.finalMatrix = joint.matrix;

            // TODO this is awkward
            break;
        }
    }

    file->position(sectionStart + sectionSize);
}

void BmdFile::readSHP1()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    uint16_t numBatches = file->readShort();
    file->skip(0x2);

    uint32_t batchesOffset = file->readInt();
    file->skip(0x8);

    uint8_t batchAttribsOffset = file->readInt();
    uint8_t matrixTableOffset = file->readInt();
    uint8_t dataOffset = file->readInt();
    uint8_t matrixDataOffset = file->readInt();
    uint8_t pktLocationsOffset = file->readInt();

    m_batches.clear();
    m_batches.reserve(numBatches);

    for(uint32_t i = 0; i < numBatches; i++)
    {
        Batch& batch = m_batches[i];

        file->position(sectionStart + batchesOffset + (i * 0x28));

        batch.matrixType = file->readByte();
        file->skip(0x1);

        // TODO why & 0xFFFF for a short?
        uint16_t numPackets = file->readShort() & 0xFFFF;
        uint16_t attribsOffset = file->readShort() & 0xFFFF;
        uint16_t firstMatrixIndex = file->readShort() & 0xFFFF;
        uint16_t firstPktIndex = file->readShort() & 0xFFFF;

        file->skip(0x2);
        batch.unk = file->readFloat();

        std::vector<uint32_t> attribs;
        file->position(sectionStart + batchAttribsOffset + attribsOffset);

        uint32_t arrayMask = 0;

        while(true)
        {
            uint32_t arrayType = file->readInt();
            uint32_t dataType = file->readInt();

            if(arrayType == 0xFF)
                break;

            uint32_t attrib = (arrayType & 0xFF) | ((dataType & 0xFF) << 8);
            attribs.push_back(attrib);

            arrayMask |= 0b1 << arrayType;
        }

        batch.packets.clear();
        batch.packets.reserve(numPackets);

        for(uint32_t j = 0; j < numPackets; j++)
        {
            Batch::Packet& packet = batch.packets[j];
            packet.primitives.clear();

            file->position(sectionStart + matrixDataOffset + ((firstMatrixIndex + j) * 0x8) + 0x2);

            uint16_t matrixTableSize = file->readShort();
            uint32_t matrixTableFirstIndex = file->readInt();

            packet.matrixTable.clear();
            packet.matrixTable.reserve(matrixTableSize);

            file->position(sectionStart + matrixTableOffset + (matrixTableFirstIndex * 0x2));
            for (int k = 0; k < matrixTableSize; k++)
                packet.matrixTable[k] = file->readShort();

            file->position(sectionStart + pktLocationsOffset + ((firstPktIndex + j) * 0x8));

            uint32_t pktSize = file->readInt();
            uint32_t pktOffset = file->readInt();

            file->position(sectionStart + dataOffset + pktOffset);
            uint32_t packetEnd = file->position() + pktSize;

            while(true)
            {
                if(file->position() >= packetEnd)
                    break;

                // TODO why & 0xFF?
                uint32_t primitiveType = file->readByte() & 0xFF;
                if(primitiveType == 0)
                    break;

                uint16_t numVertices = file->readShort();
                Batch::Packet::Primitive primitive;

                primitive.colorIndices.clear();
                primitive.colorIndices.reserve(2);

                primitive.texcoordIndices.clear();
                primitive.texcoordIndices.reserve(8);

                primitive.arrayMask = arrayMask;
                primitive.numIndices = numVertices;

                if (arrayMask &  1       ) primitive.posMatrixIndices = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 <<  9)) primitive.positionIndices = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 10)) primitive.normalIndices = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 11)) primitive.colorIndices[0] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 12)) primitive.colorIndices[1] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 13)) primitive.texcoordIndices[0] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 14)) primitive.texcoordIndices[1] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 15)) primitive.texcoordIndices[2] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 16)) primitive.texcoordIndices[3] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 17)) primitive.texcoordIndices[4] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 18)) primitive.texcoordIndices[5] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 19)) primitive.texcoordIndices[6] = std::vector<uint32_t>(numVertices);
                if (arrayMask & (1 << 20)) primitive.texcoordIndices[7] = std::vector<uint32_t>(numVertices);

                primitive.primitiveType = primitiveType;

                for(uint32_t k = 0; k < numVertices; k++)
                {
                    for(uint32_t attrib : attribs)
                    {
                        uint32_t val = 0;

                        switch(attrib & 0xFF00)
                        {
                            case 0x0000:
                            case 0x0100:
                            {
                                val = file->readByte() & 0xFF; // TODO why & 0xFF?
                                break;
                            }
                            case 0x0200:
                            case 0x0300:
                            {
                                val = file->readShort() & 0xFFFF; // TODO why & 0xFFFF?
                                break;
                            }
                            default:
                                assert(false); //Bmd: unsupported index attrib

                        }

                        switch(attrib & 0x00FF)
                        {
                            case 0:
                            {
                                primitive.posMatrixIndices[k] = val / 3;
                                break;
                            }
                            case 9:
                            {
                                primitive.positionIndices[k] = val;
                                break;
                            }
                            case 10:
                            {
                                primitive.normalIndices[k] = val;
                                break;
                            }
                            case 11:
                            case 12:
                            {
                                primitive.colorIndices[(attrib & 0xFF) - 11][k] = val;
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
                                primitive.texcoordIndices[(attrib & 0xFF) - 13][k] = val;
                                break;
                            }

                            default:
                                assert(false); // Bmd: unsupported index attrib
                        }
                    }
                }

                packet.primitives.push_back(primitive);
            }
        }
    }

    file->position(sectionStart + sectionSize);
}

void BmdFile::readMAT3()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    uint16_t materialCount = file->readShort();

    file->skip(0x2);
    uint32_t materialEntryTableOffsets = file->readInt();
    uint32_t remapTableOffset = file->readInt();

    std::vector<uint16_t> remapTable;
    for(uint32_t i = 0; i < materialCount; i++)
    {
        file->position(remapTableOffset + i * 0x02);
        remapTable.push_back(file->readShort());
    }

    uint32_t nameTableOffset = file->readInt();
    uint16_t nameTableStringCount = file->readShort();
    uint32_t nameTableIndex = 0x04;

    std::vector<QString> nameTable;
    for(uint32_t i = 0; i < nameTableStringCount; i++)
    {
        // const hash = view.getUint16(tableIdx + 0x00);
        file->position(sectionStart + nameTableOffset + nameTableIndex + 0x02);
        uint16_t stringOffset = file->readShort();

        file->position(sectionStart + nameTableOffset + stringOffset);
        QString string = file->readString(0, "UTF-8");
        nameTable.push_back(string);
        nameTableIndex += 0x04;
    }


    file->position(sectionStart + 0x18);
    uint32_t indirectTableOffset = file->readInt();
    uint32_t cullModeTableOffset = file->readInt();
    uint32_t materialColorTableOffset= file->readInt();
    uint32_t colorChanCountTableOffset= file->readInt();
    uint32_t colorChanTableOffset= file->readInt();
    uint32_t ambientColorTableOffset= file->readInt();

    file->skip(0xC);
    uint32_t texGenTableOffset= file->readInt();
    uint32_t postTexGenTableOffset= file->readInt();
    uint32_t texMtxTableOffset= file->readInt();
    uint32_t postTexMtxTableOffset= file->readInt();
    uint32_t textureTableOffset= file->readInt();
    uint32_t tevOrderTableOffset= file->readInt();
    uint32_t colorRegisterTableOffset= file->readInt();
    uint32_t colorConstantTableOffset= file->readInt();

    file->skip(0x4);
    uint32_t tevStageTableOffset= file->readInt();
    uint32_t tevSwapModeInfoOffset= file->readInt();
    uint32_t tevSwapModeTableInfoOffset = file->readInt();
    uint32_t fogInfoTableOffset= file->readInt();
    uint32_t alphaTestTableOffset= file->readInt();
    uint32_t blendModeTableOffset= file->readInt();
    uint32_t zModeTableOffset= file->readInt();

    m_materials.clear();

    for(uint32_t i = 0; i < materialCount; i++)
    {
        uint32_t index = i;
        QString& name = nameTable[i];
        uint32_t materialEntryIndex = materialEntryTableOffsets + (0x014C * remapTable[i]);

        file->position(sectionStart + materialEntryIndex);
        uint8_t materialMode = file->readByte();

        // Bitfield:
        //   0n001: OPA (Opaque)
        //   0b010: EDG (TexEdge / Masked)
        //   0b100: XLU (Translucent)
        // EDG has never been seen, so ignore it
        assert(materialMode == 0b001 || materialMode == 0b100);

        uint8_t cullModeIndex = file->readByte();
        uint8_t colorChanNumIndex = file->readByte();
        file->skip(0x3);
        uint8_t zModeIndex = file->readByte();

        std::vector<QColor> colorMatRegs;
        for(uint32_t j = 0; j < 2; j++)
        {

            file->position(sectionStart + materialEntryIndex + 0x08 + j * 0x02);
            uint16_t matColorIndex = file->readShort();

            if (matColorIndex != 0xFFFF)
            {
                file->position(sectionStart + materialColorTableOffset + matColorIndex * 0x04);
                colorMatRegs.push_back(readColor_RGBA8());
            }
            else
            {
                colorMatRegs.push_back(QColorConstants::White);
            }
        }

        std::vector<QColor> colorAmbRegs;
        for(uint32_t j = 0; j < 2; j++)
        {

            file->position(sectionStart + materialEntryIndex + 0x14 + j * 0x02);
            uint16_t ambColorIndex = file->readShort();

            if (ambColorIndex != 0xFFFF)
            {
                file->position(sectionStart + ambientColorTableOffset + ambColorIndex * 0x04);
                colorAmbRegs.push_back(readColor_RGBA8());
            }
            else
            {
                colorAmbRegs.push_back(QColorConstants::White);
            }
        }

        file->position(sectionStart + colorChanCountTableOffset + colorChanNumIndex);
        uint8_t lightChannelCount = file->readByte();
        std::vector<GX::LightChannelControl> lightChannels;
        for(uint32_t j = 0; j < lightChannelCount; j++)
        {
            file->position(sectionStart + materialEntryIndex + 0x0C + (j * 2) * 0x02);

            GX::ColorChannelControl colorChannel = readColorChannel(sectionStart + colorChanTableOffset, file->readShort());

            file->position(sectionStart + materialEntryIndex + 0x0C + (j * 2 + 1) * 0x02);
            GX::ColorChannelControl alphaChannel = readColorChannel(sectionStart + colorChanTableOffset, file->readShort());

            lightChannels.push_back({ colorChannel, alphaChannel });
        }

        // TODO continue https://github.com/magcius/noclip.website/blob/master/src/Common/JSYSTEM/J3D/J3DLoader.ts#L737
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
    return QColor(r, g, b, a);
}

QColor BmdFile::readColor_RGBX8()
{
    int r = file->readByte() & 0xFF;
    int g = file->readByte() & 0xFF;
    int b = file->readByte() & 0xFF;
    file->readByte();
    return QColor(r, g, b);
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

GX::ColorChannelControl BmdFile::readColorChannel(uint32_t absoluteColorChanTableOffset, uint16_t colorChanIndex) {
    if (colorChanIndex != 0xFFFF) {
        file->position(absoluteColorChanTableOffset + colorChanIndex * 0x08);
        bool lightingEnabled = file->readByte();
        //assert(lightingEnabled < 2);

        GX::ColorSrc matColorSource = GX::ColorSrc(file->readByte());
        uint8_t litMask = file->readByte();
        GX::DiffuseFunction diffuseFunction = GX::DiffuseFunction(file->readByte());

        uint8_t attnFn = file->readByte();

        GX::AttenuationFunction attenuationFunction;
        switch(attnFn)
        {
            case 0:
            case 2:
                attenuationFunction = GX::AttenuationFunction::NONE;
                break;
            case 1:
                attenuationFunction = GX::AttenuationFunction::SPEC;
                break;
            case 3:
                attenuationFunction = GX::AttenuationFunction::SPOT;
                break;
            default:
                assert(false); // invalid attnFn
        }

        GX::ColorSrc ambColorSource = GX::ColorSrc(file->readByte());

        return { lightingEnabled, matColorSource, ambColorSource, litMask, diffuseFunction, attenuationFunction };
    } else {
        bool lightingEnabled = false;
        GX::ColorSrc matColorSource = GX::ColorSrc::REG;
        uint8_t litMask = 0;
        GX::DiffuseFunction diffuseFunction = GX::DiffuseFunction::CLAMP;
        GX::AttenuationFunction attenuationFunction = GX::AttenuationFunction::NONE;
        GX::ColorSrc ambColorSource = GX::ColorSrc::REG;
        return { lightingEnabled, matColorSource, ambColorSource, litMask, diffuseFunction, attenuationFunction };
    }
}

glm::vec3 BmdFile::readVec3()
{
    return glm::vec3(
        file->readFloat(),
        file->readFloat(),
        file->readFloat()
    );
}
