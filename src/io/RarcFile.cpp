#include "io/RarcFile.h"

#include <stack>

#include "io/Yaz0File.h"
#include "io/InRarcFile.h"
#include "Util.h"

RarcFile::RarcFile(const QString& rarcFilePath) : m_filePath(rarcFilePath)
{
    file = new Yaz0File(rarcFilePath);
    file->setBigEndian(true);

    uint32_t magic = file->readInt();
    assert(magic == 0x52415243); // File signature is wrong

    file->position(0xC);
    uint32_t fileDataOffset = file->readInt() + 0x20;
    file->position(0x20);

    uint32_t numDirNodes = file->readInt();
    dirEntries.reserve(numDirNodes);
    uint32_t dirNodesOffset = file->readInt() + 0x20;

    uint32_t numFileEntries = file->readInt();
    fileEntries.reserve(numFileEntries);
    uint32_t fileEntriesOffset = file->readInt() + 0x20;

    file->skip(0x4);

    uint32_t stringTableOffset = file->readInt() + 0x20;
    m_unk38 = file->readInt();

    DirEntry* root = new DirEntry();

    file->position(dirNodesOffset + 0x06);
    uint16_t rnOffset = file->readShort();

    file->position(stringTableOffset + rnOffset);
    root->name = file->readString(); // TODO ASCII
    root->fullName = '/' + root->name;
    root->tempID = 0;

    dirEntries.insert(std::make_pair("/", root));

    for(int i = 0; i < numDirNodes; i++)
    {
        DirEntry* parentDir;
        for(auto& [key, entry] : dirEntries)
        {
            if(entry->tempID == i)
            {
                parentDir = entry;
                break;
            }
        }

        file->position(dirNodesOffset + (i * 0x10) + 10);

        uint16_t numEntries = file->readShort();
        uint32_t firstEntry = file->readInt();
        for(int j = 0; j < numEntries; j++)
        {
            uint32_t entryOffset = fileEntriesOffset + ((j + firstEntry) * 0x14);
            file->position(entryOffset);
            file->skip(0x4);

            uint32_t entryType = file->readShort() & 0xFFFF; // TODO why this AND and not u16?
            uint32_t nameOffset = file->readShort() & 0xFFFF;
            uint32_t dataOffset = file->readInt();
            uint32_t dataSize = file->readInt();

            file->position(stringTableOffset + nameOffset);
            QString name = file->readString(); // TODO ASCII
            if(name == "." || name == "..")
                continue;

            QString fullName = parentDir->fullName + '/' + name;

            if(entryType == 0x0200)
            {
                DirEntry* d = new DirEntry{
                    parentDir,
                    name,
                    fullName,
                    dataOffset
                };

                dirEntries.insert(std::make_pair(pathToKey(fullName), d));
                parentDir->childrenDirs.push_back(d);
            }
            else
            {
                FileEntry* f = new FileEntry{
                    parentDir,
                    fileDataOffset + dataOffset,
                    dataSize,
                    name,
                    fullName
                };

                fileEntries.insert(std::make_pair(pathToKey(fullName), f));
                parentDir->childrenFiles.push_back(f);
            }
        }
    }
}

void RarcFile::save()
{
    // load all files that haven't been read yet
    for(auto& [key, fileEntry] : fileEntries)
    {
        if(!fileEntry->data.isEmpty())
            continue;

        file->position(fileEntry->dataOffset);
        fileEntry->data = file->readBytes(fileEntry->dataSize);
    }

    uint32_t dirOffset = 0x40;
    uint32_t fileOffset = dirOffset + align32(dirEntries.size() * 0x10);
    uint32_t stringOffset = fileOffset + align32((fileEntries.size() + (dirEntries.size() * 3) - 1) * 0x14);

    uint32_t dataOffset = stringOffset;
    uint32_t dataLength = 0;

    for(const auto& [key, dirEntry] : dirEntries)
        dataOffset += dirEntry->name.length() + 1;

    for(const auto& [key, fileEntry] : fileEntries)
    {
        dataOffset += fileEntry->name.length() + 1;
        dataLength += align32(fileEntry->dataSize);
    }
    dataOffset += 0x5;
    dataOffset = align32(dataOffset);

    uint32_t dirSubOffset = 0, fileSubOffset = 0, stringSubOffset = 0, dataSubOffset = 0;

    file->setLength(dataOffset + dataLength);


    // Write RARC header (some parts we go back to later)
    file->position(0);
    file->writeInt(0x52415243); // RARC magic
    file->writeInt(dataOffset + dataLength);
    file->writeInt(0x00000020);
    file->writeInt(dataOffset - 0x20);
    file->writeInt(dataLength);
    file->writeInt(dataLength);
    file->writeInt(0x00000000); // TODO we should skip here
    file->writeInt(0x00000000);
    file->writeInt(dirEntries.size());
    file->writeInt(dirOffset - 0x20);
    file->writeInt(fileEntries.size() + (dirEntries.size() * 3) - 1);
    file->writeInt(fileOffset - 0x20);
    file->writeInt(dataOffset - stringOffset);
    file->writeInt(stringOffset - 0x20);
    file->writeInt(m_unk38);
    file->writeInt(0x00000000); // TODO we should skip here

    file->position(stringOffset);
    file->writeString("."); // TODO ASCII
    file->writeString(".."); // TODO ASCII
    stringSubOffset += 0x5;

    std::stack<Iterator<DirEntry*>> dirStack;

    std::vector<DirEntry*> dirEntriesVector;
    dirEntriesVector.reserve(dirEntries.size());
    for(auto& [key, dirEntry] : dirEntries)
        dirEntriesVector.push_back(dirEntry);

    DirEntry* curDir = dirEntriesVector[0];
    int c = 1;
    while(curDir->parentDir != nullptr)
        curDir = dirEntriesVector[c++]; // haha c++

    uint16_t fileID = 0;
    while(true)
    {
        // write the directory node
        curDir->tempID = dirSubOffset / 0x10;
        file->position(dirOffset + dirSubOffset);
        file->writeInt((curDir->tempID == 0) ? 0x524F4F54 : dirMagic(curDir->name));
        file->writeInt(stringSubOffset);
        file->writeShort(nameHash(curDir->name));
        file->writeShort(2 + curDir->childrenDirs.size() + curDir->childrenFiles.size()); // acount for . and ..
        file->writeInt(fileSubOffset / 0x14);
        dirSubOffset += 0x10;

        if(curDir->tempID > 0)
        {
            file->position(curDir->tempNameOffset);
            file->writeShort(stringSubOffset);
            file->writeInt(curDir->tempID);
        }

        file->position(stringOffset + stringSubOffset);
        stringSubOffset += file->writeString(curDir->name); // TODO ASCII

        // write the child file & dir entries
        file->position(fileOffset + fileSubOffset);
        for(DirEntry* dirEntry : curDir->childrenDirs)
        {
            file->writeShort(0xFFFF);
            file->writeShort(nameHash(dirEntry->name));
            file->writeShort(0x0200);
            dirEntry->tempNameOffset = file->position();
            file->skip(0x6);
            file->writeInt(0x00000010);
            file->writeInt(0x00000000); // TODO skip?
            fileSubOffset += 0x14;
        }

        for(FileEntry* fileEntry : curDir->childrenFiles)
        {
            file->position(fileOffset + fileSubOffset);
            file->writeShort(fileID);
            file->writeShort(nameHash(fileEntry->name));
            file->writeShort(0x1100);
            file->writeShort(stringSubOffset);
            file->writeInt(dataSubOffset);
            file->writeInt(fileEntry->dataSize);
            file->writeInt(0x00000000); // TODO skip?
            fileSubOffset += 0x14;

            // make sure every file has a unique ID
            fileID++;

            file->position(stringOffset + stringSubOffset);
            stringSubOffset += file->writeString(fileEntry->name); // TODO ASCII

            file->position(dataOffset + dataSubOffset);
            fileEntry->dataOffset = file->position();

            // TODO can we just initialize it with fileEntry->data?
            QByteArray fileData(fileEntry->dataSize, 0);
            fileData.replace(0, fileEntry->data.size(), fileEntry->data);

            file->writeBytes(fileData);
            dataSubOffset += align32(fileEntry->dataSize);

            // release RAM
            fileEntry->data.clear();
        }

        file->position(fileOffset + fileSubOffset);
        file->writeShort(0xFFFF);
        file->writeShort(0x002E);
        file->writeShort(0x0200);
        file->writeShort(0x0000);
        file->writeInt(curDir->tempID);
        file->writeInt(0x00000010);
        file->writeInt(0x00000000);
        file->writeShort(0xFFFF);
        file->writeShort(0x00B8);
        file->writeShort(0x0200);
        file->writeShort(0x0002);
        file->writeInt((curDir->parentDir != nullptr) ? curDir->parentDir->tempID : 0xFFFFFFFF);
        file->writeInt(0x00000010);
        file->writeInt(0x00000000);
        fileSubOffset += 0x28;

        /**
         * determine who's next on the list
         * if we have a child directory, process it
         * otherwise, look if we have remaining siblings
         * and if none, go back to our parent and look for siblings again
         * until we have done them all
        **/
        if(!curDir->childrenDirs.empty())
        {
            dirStack.push(Iterator(&curDir->childrenDirs));
            curDir = *(++(dirStack.top())); // TODO might crash, no idea
        }
        else
        {
            curDir = nullptr;
            while(curDir == nullptr)
            {
                if(dirStack.empty())
                    break;

                auto& iterator = dirStack.top();
                if(iterator.hasNext())
                    curDir = *(++iterator);
                else
                    dirStack.pop();
            }

            if(curDir == nullptr)
                break;
        }
    }

    file->save();
}

void RarcFile::close()
{
    file->close();
}

QStringList RarcFile::getSubDirectories(const QString& dirName)
{
    QStringList ret;
    if(!directoryExists(dirName))
        return ret;

    DirEntry* dir = dirEntries[pathToKey(dirName)];

    for(auto dirEntry : dir->childrenDirs)
        ret.push_back(dirEntry->name);

    return ret;
}

bool RarcFile::directoryExists(const std::string& dirName)
{
    return directoryExists(QString::fromStdString(dirName));
}

bool RarcFile::directoryExists(const QString& dirName)
{
    return dirEntries.find(pathToKey(dirName)) != dirEntries.end();
}

void RarcFile::mkDir(const QString& parent, const QString& dirName)
{
    QString fullName = parent + '/' + dirName;

    // do nothing if parent doesn't exist, or dir exists
    if(!directoryExists(parent) || directoryExists(fullName))
        return;

    DirEntry* parentDir = dirEntries[parent.toLower().toStdString()];
    DirEntry* newDir = new DirEntry();
    newDir->fullName = fullName;
    newDir->name = dirName;
    newDir->parentDir = parentDir;
    parentDir->childrenDirs.push_back(newDir);
    dirEntries.insert(std::make_pair(fullName.toLower().toStdString(), newDir));
}

void RarcFile::mvDir(const QString& oldName, const QString& newName)
{
    if(!directoryExists(oldName))
        return;

    DirEntry* victimDir = dirEntries[oldName.toLower().toStdString()];
    DirEntry* parent = victimDir->parentDir;

    QString newFullName = parent->fullName + '/' + newName;

    QString parentPath;
    if(parent != nullptr)
    {
        if(fileExists(newFullName) || directoryExists(newFullName))
            return;

        parentPath = parent->fullName;
    }

    dirEntries.erase(victimDir->fullName.toLower().toStdString());
    victimDir->name = newName;
    victimDir->fullName = newFullName;
    dirEntries.insert(std::make_pair(victimDir->fullName.toLower().toStdString(), victimDir));
}

void RarcFile::rmDir(const QString& dirName)
{
    if(!directoryExists(dirName))
        return;

    DirEntry* victimDir = dirEntries[dirName.toLower().toStdString()];
    DirEntry* parent = victimDir->parentDir;

    // TODO if(parent != nullptr)
        //parent->childrenDirs.erase(parent->childrenDirs[]);

}

QStringList RarcFile::getFiles(const QString& dirName)
{
    QStringList ret;
    if(!directoryExists(QString::fromStdString(pathToKey(dirName))))
        return ret;

    DirEntry* dir = dirEntries[pathToKey(dirName)];

    for(FileEntry* file : dir->childrenFiles)
        ret.push_back(file->name);

    return ret;
}

bool RarcFile::fileExists(const std::string& filePath)
{
    return fileExists(QString::fromStdString(filePath));
}


bool RarcFile::fileExists(const QString& filePath)
{
    return fileEntries.find(pathToKey(filePath)) != fileEntries.end();
}

void RarcFile::mkFile(const QString& dirName, const QString& fileName)
{
    // TODO fix this mess of mixing std::string with QString
    QString fullName = dirName + '/' + fileName;
    std::string parentKey = pathToKey(dirName);
    std::string fnKey = pathToKey(fullName);

    if(!directoryExists(parentKey)
            || fileExists(fnKey)
            || directoryExists(fnKey))
        return;

    DirEntry* parentDir = dirEntries[parentKey];

    FileEntry* fileEntry = new FileEntry
    {
        parentDir,
        0, // dataOffset is not set in Whitehole??
        0, // dataSize starts at zero
        fileName,
        fullName,
    };

    parentDir->childrenFiles.push_back(fileEntry);
    fileEntries.insert(std::make_pair(pathToKey(fullName), fileEntry));
}

void RarcFile::mvFile(const QString& oldPath, const QString& newPath)
{
    std::string oldFile = pathToKey(oldPath);
    if(!fileExists(oldFile))
        return;

    FileEntry* fileEntry = fileEntries[oldFile];
    DirEntry* parent= fileEntry->parentDir;

    QString newFullName = parent->fullName + '/' + newPath;
    std::string parentKey = pathToKey(newFullName);
    if(fileExists(parentKey)
        || directoryExists(parentKey))
        return; // TODO Whitehole says "temp" here. Why?

    std::string fnKey = pathToKey(fileEntry->fullName);
    fileEntries.erase(fnKey);

    fileEntry->name = newPath;
    fileEntry->fullName = newFullName;

    fnKey = pathToKey(fileEntry->fullName);
    fileEntries.insert(std::make_pair(fnKey, fileEntry));
}

void RarcFile::rmFile(const QString& filePath)
{
    std::string file = pathToKey(filePath);
    if(!fileExists(file))
        return;

    FileEntry* fileEntry = fileEntries[file];
    DirEntry* parent = fileEntry->parentDir;

    // I wish this were easier in C++ :weary:
    for(int i = 0; i < parent->childrenFiles.size(); i++)
    {
        if(parent->childrenFiles[i] == fileEntry)
        {
            parent->childrenFiles.erase(parent->childrenFiles.begin() + i);
            break;
        }
    }

    fileEntries.erase(file);
}

FileBase* RarcFile::openFile(const QString& filePath)
{
    if(!fileExists(pathToKey(filePath)))
        return nullptr; // TODO error?

    return new InRarcFile(this, filePath);
}

QByteArray RarcFile::getFileContents(const QString& filePath)
{
    FileEntry* fileEntry = fileEntries[pathToKey(filePath)];

    if(fileEntry->data != nullptr)
        return fileEntry->data;

    file->position(fileEntry->dataOffset);
    return file->readBytes(fileEntry->dataSize); // TODO should we set fileEntry->data?
}

void RarcFile::reinsertFile(const InRarcFile& file)
{
    FileEntry* fileEntry = fileEntries[pathToKey(file.m_fullName)];
    fileEntry->data = file.getContents();
    fileEntry->dataSize = file.getLength(); // TODO maybe unnecessary, could use data length directly
}


std::string RarcFile::pathToKey(const QString& path)
{
    std::string ret = path.toLower().toStdString();
    ret = ret.substr(1);
    if (ret.find("/") == std::string::npos)
        return "/";

    ret = ret.substr(ret.find("/"));
    return ret;
}

uint32_t RarcFile::align32(uint32_t val) {
    return (val + 0x1F) & ~0x1F;
}

uint32_t RarcFile::dirMagic(const QString& name) {
    std::string upperName = name.toUpper().toStdString();
    int ret = 0;

    for (int i = 0; i < 4; i++) {
        ret <<= 8;

        if (i >= upperName.length())
            ret += 0x20;
        else
            ret += upperName[i];
    }

    return ret;
}

uint16_t RarcFile::nameHash(const QString& name) {
    uint16_t ret = 0;
    for (char ch : name.toStdString()) {
        ret *= 3;
        ret += ch;
    }

    return ret;
}














