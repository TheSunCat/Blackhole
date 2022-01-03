#pragma once
#include <QString>
#include <QByteArray>
#include <unordered_map>
#include <vector>
#include <string>

#include "FileBase.h"

class QStringList;

class RarcFile
{
    static std::string pathToKey(const QString& path);
    static uint32_t align32(uint32_t val);
    static uint32_t dirMagic(const QString& name);
    static uint16_t nameHash(const QString& name);


    QString m_filePath;
    FileBase* file;

    uint32_t m_unk38;

    struct DirEntry;

    struct FileEntry {
        DirEntry* parentDir;

        uint32_t dataOffset;
        uint32_t dataSize;

        QString name;
        QString fullName;

        QByteArray data;
    };

    struct DirEntry {
        DirEntry* parentDir;

        QString name;
        QString fullName;

        uint32_t tempID;
        uint32_t tempNameOffset;

        std::vector<DirEntry*> childrenDirs;
        std::vector<FileEntry*> childrenFiles;
    };


    std::unordered_map<std::string, DirEntry*> dirEntries;
    std::unordered_map<std::string, FileEntry*> fileEntries;

public:
    RarcFile(const QString& rarcFilePath);

    void save();
    void close();

    QStringList getSubDirectories(const QString& dirName);
    bool directoryExists(const QString& dirName);
    void mkDir(const QString& parent, const QString& dirName);
    void mvDir(const QString& oldName, const QString& newName);
    void rmDir(const QString& dirName);


    QStringList getFiles();
    bool fileExists(const QString& fileName);
    void mkFile(const QString& fileName);
    void mvFile(const QString& fileName, const QString& destination);
    void rmFile(const QString& fileName);

    FileBase* openFile(QString filename);
};
