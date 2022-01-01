#pragma once
#include <QString>

#include "FileBase.h"

class QStringList;

class RarcFile
{
    QString filePath;
    FileBase* file;

public:
    RarcFile(QString rarcFilePath);

    void save();
    void close();

    QStringList getDirectories();
    bool directoryExists();
    void mkDir(QString dirName);
    void mvDir(QString dirName, QString destination);
    void rmDir(QString dirName);


    QStringList getFiles();
    bool fileExists();
    void mkFile(QString fileName);
    void mvFile(QString fileName, QString destination);
    void rmFile(QString fileName);

    FileBase* openFile(QString filename);
};
