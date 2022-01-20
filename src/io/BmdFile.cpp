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
        if(section == "MAT3")
            readMAT3();
        if(section == "MDL3")
            readMDL3();
        if(section == "TEX1")
            readTEX1();
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


// huge thanks to noclip.website for this parser!
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
    std::vector<QString> nameTable = readStringTable(sectionStart + nameTableOffset);


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

        std::vector<GX::TexGen> texGens;
        for(uint32_t j = 0; j < 8; j++)
        {
            file->position(sectionStart + materialEntryIndex + 0x28 + j * 0x02);
            int16_t texGenIndex = *(int16_t*)file->readShort(); // type punning. bad?

            if(texGenIndex < 0)
                continue; // negative index means skip

            file->position(sectionStart + texGenTableOffset + texGenIndex * 0x04);
            GX::TexGenType type = GX::TexGenType(file->readByte());
            GX::TexGenSrc source = GX::TexGenSrc(file->readByte());
            GX::TexGenMatrix matrixCheck = GX::TexGenMatrix(file->readByte());
            assert(file->readByte() == 0xFF);

            GX::PostTexGenMatrix postMatrix = GX::PostTexGenMatrix::PTIDENTITY;

            file->position(sectionStart + materialEntryIndex + 0x38 + j * 0x02);
            int16_t postTexGenIndex = *(int16_t*)file->readShort(); // type punning. bad?
            if(postTexGenTableOffset > 0 && postTexGenIndex >= 0)
            {
                file->position(sectionStart + postTexGenTableOffset + texGenIndex * 0x04 + 0x02);
                postMatrix = GX::PostTexGenMatrix(file->readByte());
                assert(file->readByte() == 0xFF);
            }

            // BTK can apply texture animations to materials that have the matrix set to IDENTITY.
            // For this reason, we always assign a texture matrix. In theory, the file should
            // have an identity texture matrix in the texMatrices section, so it should render correctly
            GX::TexGenMatrix matrix = GX::TexGenMatrix(int(GX::TexGenMatrix::TEXMTX0) + j * 3);

            // If we ever find a counter-example for this, I'll have to rethink the scheme, but I
            // *believe* that texture matrices should always be paired with TexGens in order.
            assert(matrixCheck == GX::TexGenMatrix::IDENTITY || matrixCheck == matrix);

            bool normalize = false;
            texGens.push_back({ type, source, matrix, normalize, postMatrix });
        }

        std::vector<TexMatrix*> texMatrices;
        for(uint32_t j = 0; j < 10; j++)
        {
            file->position(sectionStart + materialEntryIndex + 0x48 + j * 0x02);
            int16_t texMatrixIndex = *(int16_t*)file->readShort();

            if(texMtxTableOffset > 0 && texMatrixIndex >= 0)
            {
                uint32_t texMtxOffset = texMtxTableOffset + texMatrixIndex * 0x64;
                file->position(sectionStart + texMtxOffset);

                TexMatrixProjection projection = TexMatrixProjection(file->readByte());
                uint8_t info = file->readByte();

                GX::TexMtxMapMode matrixMode = GX::TexMtxMapMode(info & 0x3F);

                // Detect uses of unlikely map modes.
                assert(matrixMode != GX::TexMtxMapMode::ProjmapBasic && matrixMode != GX::TexMtxMapMode::ViewProjmapBasic &&
                        int(matrixMode) != 0x04 && int(matrixMode) != 0x05);

                assert(file->readShort() == 0xFFFF);

                float centerS = file->readFloat();
                float centerT = file->readFloat();
                float centerQ = file->readFloat();

                float scaleS = file->readFloat();
                float scaleT = file->readFloat();

                float rotation = file->readShort() / 0x7FFF;
                assert(file->readShort() == 0xFFFF);

                float translationS = file->readFloat();
                float translationT = file->readFloat();

                // TODO is this the right order?
                glm::mat4 effectMatrix(
                    file->readFloat(), file->readFloat(), file->readFloat(), file->readFloat(),
                    file->readFloat(), file->readFloat(), file->readFloat(), file->readFloat(),
                    file->readFloat(), file->readFloat(), file->readFloat(), file->readFloat(),
                    file->readFloat(), file->readFloat(), file->readFloat(), file->readFloat()

                );

                glm::mat4 matrix(1.0f);
                bool isMaya = info >> 7;
                if(isMaya)
                {
                    float theta = rotation * M_PI;
                    float sinR = sin(theta);
                    float cosR = cos(theta);

                    matrix[0][0]  = scaleS *  cosR;
                    matrix[1][0]  = scaleS *  sinR;
                    matrix[3][0]  = scaleS * ((-0.5 * cosR) - (0.5 * sinR - 0.5) - translationS);

                    matrix[0][1]  = scaleT * -sinR;
                    matrix[1][1]  = scaleT *  cosR;
                    matrix[3][1]  = scaleT * ((-0.5 * cosR) + (0.5 * sinR - 0.5) + translationT) + 1.0;
                }
                else
                {
                    float theta = rotation * M_PI;
                    float sinR = sin(theta);
                    float cosR = cos(theta);

                    matrix[0][0]  = scaleS *  cosR;
                    matrix[1][0]  = scaleS * -sinR;
                    matrix[3][0] = translationS + centerS - (matrix[0][0] * centerS + matrix[1][0] * centerT);

                    matrix[0][1]  = scaleT *  sinR;
                    matrix[1][1]  = scaleT *  cosR;
                    matrix[3][1]  = translationT + centerT - (matrix[1][1] * centerS + matrix[1][1] * centerT);
                }

                texMatrices.push_back(new TexMatrix{ info, projection, effectMatrix, matrix });
            }
            else
            {
                texMatrices.push_back(nullptr);
            }
        }

        // Since texture matrices are assigned to TEV stages in order, we
        // should never actually have more than 8 of these.
        assert(texMatrices[8] == nullptr && texMatrices[9] == nullptr);

        // These are never read in actual J3D.
        /*
        const postTexMatrices: (TexMtx | null)[] = [];
        for (let j = 0; j < 20; j++) {
            const postTexMtxIndex = view.getInt16(materialEntryIdx + 0x5C + j * 0x02);
            if (postTexMtxTableOffs > 0 && postTexMtxIndex >= 0)
                postTexMatrices[j] = readTexMatrix(postTexMtxTableOffs, postTexMtxIndex);
            else
                postTexMatrices[j] = null;
        }
        */

        std::vector<int16_t> textureIndexes; // shouldn't this be indices?
        for(uint32_t j = 0; j < 8; j++)
        {
            file->position(sectionStart + materialEntryIndex + 0x84 + j * 0x02);
            uint16_t textureTableIndex = file->readShort();
            if(textureTableIndex != 0xFFFF)
            {
                file->position(sectionStart + textureTableOffset + textureTableIndex * 0x02);
                textureIndexes.push_back(file->readShort());
            }
            else
            {
                textureIndexes.push_back(-1);
            }
        }

        std::vector<QColor> colorConstants; // shouldn't this be indices?
        for(uint32_t j = 0; j < 4; j++)
        {
            file->position(sectionStart + materialEntryIndex + 0x94 + j * 0x02);
            uint16_t colorIndex = file->readShort();
            if(colorIndex != 0xFFFF)
            {
                file->position(sectionStart + colorConstantTableOffset + colorIndex * 0x04);
                colorConstants.push_back(readColor_RGBA8());
            }
            else
            {
                colorConstants.push_back(QColorConstants::White);
            }
        }

        std::vector<QColor> colorRegisters; // shouldn't this be indices?
        for(uint32_t j = 0; j < 4; j++)
        {
            file->position(sectionStart + materialEntryIndex + 0xDC + j * 0x02);
            uint16_t colorIndex = file->readShort();
            if(colorIndex != 0xFFFF)
            {
                file->position(sectionStart + colorRegisterTableOffset + colorIndex * 0x08);
                colorConstants.push_back(readColor_RGBA16());
            }
            else
            {
                colorConstants.push_back(QColorConstants::Transparent);
            }
        }

        std::vector<GX::IndTexStage> indTexStages;
        std::vector<float> indTexMatrices;
        uint32_t indirectEntryOffset = indirectTableOffset + i * 0x138;

        bool hasIndirectTable = indirectTableOffset != nameTableOffset;

        if(hasIndirectTable)
        {
            file->position(sectionStart + indirectEntryOffset);

            uint8_t hasIndirect = file->readByte();
            assert((hasIndirect & 0b11111110) == 0); // make sure it's a bool

            uint8_t indTexStageNum = file->readByte();
            assert(indTexStageNum <= 4);

            for(uint32_t j = 0; j < indTexStageNum; j++)
            {
                // SetIndTexOrder
                uint32_t indTexOrderOffset = indirectEntryOffset + 0x04 + j * 0x04;
                file->position(sectionStart + indTexOrderOffset);

                GX::TexCoordID texCoordId = GX::TexCoordID(file->readByte());
                GX::TexMapID texture = GX::TexMapID(file->readByte());

                // SetIndTexCoordScale
                uint32_t indTexScaleOffset = indirectEntryOffset + 0x04 + (0x04 * 4) + (0x1C * 3) + j * 0x04;
                file->position(sectionStart + indTexScaleOffset);

                GX::IndTexScale scaleS = GX::IndTexScale(file->readByte());
                GX::IndTexScale scaleT = GX::IndTexScale(file->readByte());
                indTexStages.push_back({ texCoordId, texture, scaleS, scaleT });

                // SetIndTexMatrix
                uint32_t indTexMatrixOffset = indirectEntryOffset + 0x04 + (0x04 * 4) + j * 0x1C;
                file->position(sectionStart + indTexMatrixOffset);

                float p00 = file->readFloat();
                float p01 = file->readFloat();
                float p02 = file->readFloat();
                float p10 = file->readFloat();
                float p11 = file->readFloat();
                float p12 = file->readFloat();
                float scale = pow(2, file->readInt());

                // TODO should this be a mat2x4?
                indTexMatrices.push_back(p00*scale);
                indTexMatrices.push_back(p01*scale);
                indTexMatrices.push_back(p02*scale);
                indTexMatrices.push_back(scale);
                indTexMatrices.push_back(p10*scale);
                indTexMatrices.push_back(p11*scale);
                indTexMatrices.push_back(p12*scale);
                indTexMatrices.push_back(0.0f);

            }
        }

        std::vector<GX::TevStage> tevStages;
        for(uint32_t j = 0; j < 16; j++)
        {
            // TevStage
            file->position(sectionStart + materialEntryIndex + 0xE4 + j * 0x02);

            int16_t tevStageIndex = *(int16_t*)file->readShort();
            if(tevStageIndex < 0)
                continue;

            uint32_t tevStageOffset = tevStageTableOffset + tevStageIndex * 0x14;
            file->position(sectionStart + tevStageOffset + 1); // skip unk byte

            GX::CC colorInA = GX::CC(file->readByte());
            GX::CC colorInB = GX::CC(file->readByte());
            GX::CC colorInC = GX::CC(file->readByte());
            GX::CC colorInD = GX::CC(file->readByte());
            GX::TevOp colorOp = GX::TevOp(file->readByte());
            GX::TevBias colorBias = GX::TevBias(file->readByte());
            GX::TevScale colorScale = GX::TevScale(file->readByte());
            bool colorClamp = file->readByte();
            GX::Register colorRegID = GX::Register(file->readByte());

            GX::CA alphaInA = GX::CA(file->readByte());
            GX::CA alphaInB = GX::CA(file->readByte());
            GX::CA alphaInC = GX::CA(file->readByte());
            GX::CA alphaInD = GX::CA(file->readByte());
            GX::TevOp alphaOp = GX::TevOp(file->readByte());
            GX::TevBias alphaBias = GX::TevBias(file->readByte());
            GX::TevScale alphaScale = GX::TevScale(file->readByte());
            bool alphaClamp = file->readByte();
            GX::Register alphaRegID = GX::Register(file->readByte());

            // TevOrder
            file->position(sectionStart + materialEntryIndex + 0xBC + j * 0x02);

            uint16_t tevOrderIndex = file->readShort();

            uint32_t tevOrderOffset = tevOrderTableOffset + tevOrderIndex * 0x04;
            file->position(sectionStart + tevOrderOffset);

            GX::TexCoordID texCoordID = GX::TexCoordID(file->readByte());
            GX::TexMapID texMap = GX::TexMapID(file->readByte());

            GX::RasColorChannelID channelID;
            switch (GX::ColorChannelID(file->readByte())) {
                case GX::ColorChannelID::COLOR0:
                case GX::ColorChannelID::ALPHA0:
                case GX::ColorChannelID::COLOR0A0:
                    channelID = GX::RasColorChannelID::COLOR0A0;
                    break;
                case GX::ColorChannelID::COLOR1:
                case GX::ColorChannelID::ALPHA1:
                case GX::ColorChannelID::COLOR1A1:
                    channelID = GX::RasColorChannelID::COLOR1A1;
                    break;
                case GX::ColorChannelID::ALPHA_BUMP:
                    channelID = GX::RasColorChannelID::ALPHA_BUMP;
                    break;
                case GX::ColorChannelID::ALPHA_BUMP_N:
                    channelID = GX::RasColorChannelID::ALPHA_BUMP_N;
                    break;
                case GX::ColorChannelID::COLOR_ZERO:
                case GX::ColorChannelID::COLOR_NULL:
                    channelID = GX::RasColorChannelID::COLOR_ZERO;
                    break;
                default:
                    assert(false);
            }
            assert(file->readByte() == 0xFF);

            // KonstSel
            file->position(sectionStart + materialEntryIndex + 0x9C + j);
            GX::KonstColorSel konstColorSel = GX::KonstColorSel(file->readByte());
            file->skip(0x10);
            GX::KonstAlphaSel konstAlphaSel = GX::KonstAlphaSel(file->readByte());

            // SetTevSwapMode
            file->position(sectionStart + materialEntryIndex + 0x104 + j * 0x02);
            uint16_t tevSwapModeIndex = file->readShort();

            file->position(sectionStart + tevSwapModeInfoOffset + tevSwapModeIndex * 0x04);
            uint8_t tevSwapModeRasSel = file->readByte();
            uint8_t tevSwapModeTexSel = file->readByte();

            file->position(sectionStart + materialEntryIndex + 0x124 + tevSwapModeRasSel * 0x02);
            uint16_t tevSwapModeTableRasIndex = file->readShort();

            file->position(sectionStart + materialEntryIndex + 0x124 + tevSwapModeTexSel * 0x02);
            uint16_t tevSwapModeTableTexIndex = file->readShort();

            file->position(sectionStart + tevSwapModeTableInfoOffset + tevSwapModeTableRasIndex * 0x04);
            uint8_t rasSwapA = file->readByte();
            uint8_t rasSwapB = file->readByte();
            uint8_t rasSwapC = file->readByte();
            uint8_t rasSwapD = file->readByte();

            file->position(sectionStart + tevSwapModeTableInfoOffset + tevSwapModeTableTexIndex * 0x04);
            uint8_t texSwapA = file->readByte();
            uint8_t texSwapB = file->readByte();
            uint8_t texSwapC = file->readByte();
            uint8_t texSwapD = file->readByte();

            GX::SwapTable rasSwapTable = {
                GX::TevColorChan(rasSwapA),
                GX::TevColorChan(rasSwapB),
                GX::TevColorChan(rasSwapC),
                GX::TevColorChan(rasSwapD)
            };

            GX::SwapTable texSwapTable = {
                GX::TevColorChan(texSwapA),
                GX::TevColorChan(texSwapB),
                GX::TevColorChan(texSwapC),
                GX::TevColorChan(texSwapD)
            };

            // SetTevIndirect
            uint32_t indTexStageOffset = indirectEntryOffset + 0x04 + (0x04 * 4) + (0x1C * 3) + (0x04 * 4) + j * 0x0C;
            GX::IndTexStageID indTexStage = GX::IndTexStageID::STAGE0;
            GX::IndTexFormat indTexFormat = GX::IndTexFormat::_8;
            GX::IndTexBiasSel indTexBiasSel = GX::IndTexBiasSel::NONE;
            GX::IndTexAlphaSel indTexAlphaSel = GX::IndTexAlphaSel::OFF;
            GX::IndTexMtxID indTexMatrix = GX::IndTexMtxID::OFF;
            GX::IndTexWrap indTexWrapS = GX::IndTexWrap::OFF;
            GX::IndTexWrap indTexWrapT = GX::IndTexWrap::OFF;
            bool indTexAddPrev = false;
            bool indTexUseOrigLOD = false;

            if(hasIndirectTable)
            {
                file->position(sectionStart + indTexStageOffset);

                indTexStage = GX::IndTexStageID(file->readByte());
                indTexFormat = GX::IndTexFormat(file->readByte());
                indTexBiasSel = GX::IndTexBiasSel(file->readByte());
                indTexMatrix = GX::IndTexMtxID(file->readByte());
                assert(indTexMatrix <= GX::IndTexMtxID::T2);

                indTexWrapS = GX::IndTexWrap(file->readByte());
                indTexWrapT = GX::IndTexWrap(file->readByte());
                indTexAddPrev = file->readByte();
                indTexUseOrigLOD = file->readByte();
                indTexAlphaSel = GX::IndTexAlphaSel(file->readByte());
            }

            tevStages.push_back(GX::TevStage{
                colorInA, colorInB, colorInC, colorInC, colorOp,
                colorBias, colorScale, colorClamp, colorRegID,

                alphaInA, alphaInB, alphaInC, alphaInD, alphaOp,
                alphaBias, alphaScale, alphaClamp, alphaRegID,

                // SetTevOrder
                texCoordID, texMap, channelID,
                konstColorSel, konstAlphaSel,

                // SetTevSwapMode / SetTevSwapModeTable
                rasSwapTable, texSwapTable,

                // SetTevIndirect
                indTexStage, indTexFormat, indTexBiasSel, indTexAlphaSel,
                indTexMatrix, indTexWrapS, indTexWrapT, indTexAddPrev, indTexUseOrigLOD

            });
        }

        // SetAlphaCompare
        file->position(sectionStart + materialEntryIndex + 0x146);
        uint16_t alphaTestIndex = file->readShort();
        uint16_t blendModeIndex = file->readShort();

        uint32_t alphaTestOffset = alphaTestTableOffset + alphaTestIndex * 0x08;
        file->position(sectionStart + alphaTestOffset);

        GX::CompareType compareA = GX::CompareType(file->readByte());
        uint8_t referenceA = file->readByte() / 0xFF; // TODO should this be a float?
        GX::AlphaOp op = GX::AlphaOp(file->readByte());
        GX::CompareType compareB = GX::CompareType(file->readByte());
        uint8_t referenceB = file->readByte() / 0xFF;
        GX::AlphaTest alphaTest = { op, compareA, referenceA, compareB, referenceB };

        // SetBlendMode
        uint32_t blendModeOffset = blendModeTableOffset + blendModeIndex * 0x04;
        file->position(sectionStart + blendModeOffset);

        GX::BlendMode blendMode = GX::BlendMode(file->readByte());
        GX::BlendFactor blendSrcFactor = GX::BlendFactor(file->readByte());
        GX::BlendFactor blendDstFactor = GX::BlendFactor(file->readByte());
        GX::LogicOp blendLogicOp = GX::LogicOp(file->readByte());

        file->position(sectionStart + cullModeTableOffset + cullModeIndex * 0x04);
        GX::CullMode cullMode = GX::CullMode(file->readInt());

        uint32_t zModeOffset = zModeTableOffset + zModeIndex * 4;
        file->position(zModeOffset);

        bool depthTest = file->readByte();
        GX::CompareType depthFunc = GX::CompareType(file->readByte());
        bool depthWrite = file->readByte();

        file->position(sectionStart + materialEntryIndex + 0x144);
        uint16_t fogInfoIndex = file->readShort();

        uint32_t fogInfoOffset = fogInfoTableOffset + fogInfoIndex * 0x2C;
        file->position(sectionStart + fogInfoOffset);

        GX::FogType fogType = GX::FogType(file->readByte());
        bool fogAdjEnabled = file->readByte();
        uint16_t fogAdjCenter = file->readShort();
        float fogStartZ = file->readFloat();
        float fogEndZ = file->readFloat();
        float fogNearZ = file->readFloat();
        float fogFarZ = file->readFloat();
        QColor fogColor = readColor_RGBA8();

        std::array<uint16_t, 10> fogAdjTable;
        for(uint32_t j = 0; j < 10; j++)
            fogAdjTable[j] = file->readShort();

        GX::FogBlock fogBlock;
        bool fogProj = uint8_t(fogType) >> 3;
        if(fogProj)
        {
            // orthographic
            fogBlock.A = (fogFarZ - fogNearZ) / (fogEndZ - fogStartZ);
            fogBlock.B = 0.0f;
            fogBlock.C = (fogStartZ - fogNearZ) / (fogEndZ - fogStartZ);
        }
        else
        {
            fogBlock.A = (fogFarZ * fogNearZ) / ((fogFarZ - fogNearZ) * (fogEndZ - fogStartZ));
            fogBlock.B = (fogFarZ) / (fogFarZ - fogNearZ);
            fogBlock.C = (fogStartZ) / (fogEndZ - fogStartZ);
        }
        fogBlock.color = fogColor;
        fogBlock.adjTable = fogAdjTable;
        fogBlock.adjCenter = fogAdjCenter;

        bool translucent = materialMode == 0x04;
        bool colorUpdate = true, alphaUpdate = false;

        GX::RopInfo ropInfo{
            fogType, fogAdjEnabled,
            depthTest, depthFunc, depthWrite,
            blendMode, blendSrcFactor, blendDstFactor, blendLogicOp,

            colorUpdate, alphaUpdate
        };

        GX::Material gxMaterial{
            name, cullMode, lightChannels, texGens, tevStages, indTexStages, alphaTest
        };

        GX::autoOptimizeMaterial(gxMaterial);

        m_materials.push_back(Material{
            index, name,
            materialMode, translucent,
            textureIndexes, texMatrices,
            indTexMatrices,
            gxMaterial,
            colorMatRegs, colorAmbRegs,
            colorConstants, colorRegisters,
            fogBlock
        });
    }

    file->position(sectionStart + sectionSize);
}

void BmdFile::readMDL3()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    // this is the most important section
    // here we're going to parse it with lots of code
    // it's really good
    // and uh

    // skip section
    file->position(sectionStart + sectionSize);
}

void BmdFile::readTEX1()
{
    uint32_t sectionStart = file->position() - 4;
    uint32_t sectionSize = file->readInt();

    uint16_t textureCount = file->readShort();
    file->skip(0x02);

    uint32_t textureHeaderOffset = file->readInt();

    uint32_t nameTableOffset = file->readInt();
    std::vector<QString> nameTable = readStringTable(sectionStart + nameTableOffset);

    std::vector<Sampler> samplers;
    std::vector<TextureData> textureDatas;
    for(uint32_t i = 0; i < textureCount; i++)
    {
        uint32_t textureIndex = textureHeaderOffset + i * 0x20;
        QString& name = nameTable[i];

        GX::BTI_Texture btiTexture = readBTI(sectionStart + textureIndex, name);

        int32_t textureDataIndex = -1;

        // Try to find existing texture data
        QByteArrayView& textureData = btiTexture.data;
        if(!textureData.isNull())
        {
            for(uint32_t j = 0; j < m_textureDatas.size(); j++)
            {
                const TextureData& curTex = m_textureDatas[j];

                if(curTex.data.isNull())
                    continue;

                if(curTex.data == textureData)
                    textureDataIndex = j;
            }
        }

        if(textureDataIndex < 0)
        {
            m_textureDatas.push_back({
                btiTexture.name,
                btiTexture.width,
                btiTexture.height,
                btiTexture.format,
                btiTexture.mipCount,
                btiTexture.data,
                btiTexture.paletteFormat,
                btiTexture.paletteData,
            });

            textureDataIndex = m_textureDatas.size() - 1;
        }

        m_samplers.push_back({
            i,
            btiTexture.name,
            btiTexture.wrapS,
            btiTexture.wrapT,
            btiTexture.minFilter,
            btiTexture.magFilter,
            btiTexture.minLOD,
            btiTexture.maxLOD,
            btiTexture.lodBias,
            textureDataIndex
        });
    }

    file->position(sectionStart + sectionSize);
}

GX::BTI_Texture BmdFile::readBTI(uint32_t absoluteStartIndex, const QString& name)
{
    file->position(absoluteStartIndex);

    GX::TexFormat format = GX::TexFormat(file->readByte());
    file->skip(0x01);

    uint16_t width = file->readShort();
    uint16_t height = file->readShort();

    GX::WrapMode wrapS = GX::WrapMode(file->readByte());
    GX::WrapMode wrapT = GX::WrapMode(file->readByte());
    file->skip(0x01);

    GX::TexPalette paletteFormat = GX::TexPalette(file->readByte());
    uint16_t paletteCount = file->readShort();
    uint32_t paletteOffset = file->readInt();
    file->skip(0x04);

    GX::TexFilter minFilter = GX::TexFilter(file->readByte());
    GX::TexFilter magFilter = GX::TexFilter(file->readByte());

    float minLOD = file->readByte() / 8.f;
    float maxLOD = file->readByte() / 8.f;
    uint8_t mipCount = file->readByte();
    file->skip(0x01);

    float lodBias = file->readShort() / 100.f;
    uint32_t dataOffset = file->readInt();

    assert(minLOD == 0);

    // TODO is this actually making a view, or is sliced(pos) making a copy of it?

    QByteArrayView data;
    if(dataOffset != 0)
        data = QByteArrayView(file->getContents().sliced(absoluteStartIndex + dataOffset));

    QByteArrayView paletteData;
    if(paletteOffset != 0)
        paletteData = QByteArrayView(file->getContents().sliced(absoluteStartIndex + paletteOffset, paletteCount * 2));

    return {
        name, format, width, height,
        wrapS, wrapT, minFilter, magFilter,
        minLOD, maxLOD, lodBias, mipCount,
        data, paletteFormat, paletteData
    };
}


std::vector<QString> BmdFile::readStringTable(uint32_t absoluteOffset)
{
    std::vector<QString> ret;

    uint16_t stringCount = file->readShort();
    uint32_t index = 0x04;
    for(uint32_t i = 0; i < stringCount; i++)
    {
        // const hash = view.getUint16(tableIdx + 0x00);
        file->position(absoluteOffset + index + 0x02);
        uint16_t stringOffset = file->readShort();

        file->position(absoluteOffset + stringOffset);
        QString string = file->readString(0, "UTF-8");
        ret.push_back(string);
        index += 0x04;
    }

    return ret;
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
    return QColor(qRgb(r, g, b));
}

QColor BmdFile::readColor_RGBA16()
{
    uint16_t r = file->readInt();
    uint16_t g = file->readInt();
    uint16_t b = file->readInt();
    uint16_t a = file->readInt();
    return QColor(qRgba(r, g, b, a));
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

BmdFile::Material::~Material()
{
    // release memory

    for(TexMatrix* texMtx : texMatrices)
        delete texMtx;
}
