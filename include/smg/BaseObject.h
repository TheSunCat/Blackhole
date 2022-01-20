#pragma once

#include <glm/glm.hpp>

#include "smg/Zone.h"
#include "io/BcsvFile.h"

class BaseObject
{
protected:
    Zone m_zone;

    QString m_dir, m_layer, m_fileName;
    //QString m_type = "general";
    //QString m_oldName; // TODO what is this?
    BcsvFile::Entry& m_data;
    int32_t m_ID = -1;

    glm::vec3 m_pos, m_rot, m_scl;

    friend class ObjectRenderer;
public:

    QString m_name;

    BaseObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry);

    // initialize an empty object
    BaseObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, const glm::vec3& pos);


    virtual int save() = 0;
};
