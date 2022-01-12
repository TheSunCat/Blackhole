#include "smg/ZoneObject.h"

#include <QStringList>

ZoneObject::ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : BaseObject(zone, dir, layer, fileName, entry)
{
    m_name = m_data.getstr("name");

    m_pos = glm::vec3(
        m_data.getf("pos_x"),
        m_data.getf("pos_y"),
        m_data.getf("pos_z")
    );

    m_rot = glm::vec3(
        m_data.getf("dir_z"),
        m_data.getf("dir_y"),
        m_data.getf("dir_x")
    );

    m_scl = glm::vec3(1);
}

// TODO *new jank?
ZoneObject::ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos)
        : BaseObject(zone, dir, layer, fileName, *new BcsvFile::Entry())
{
    m_pos = pos;
    m_rot = glm::vec3(0);
    m_scl = glm::vec3(1);

    m_data.insert("name", m_name);
}

int ZoneObject::save()
{
    m_data.insert("name", m_name);

    m_data.insert("pos_x", m_pos.x);
    m_data.insert("pos_y", m_pos.y);
    m_data.insert("pos_z", m_pos.z);

    m_data.insert("dir_x", m_pos.x);
    m_data.insert("dir_y", m_pos.y);
    m_data.insert("dir_z", m_pos.z);

    return 0;
}


ZoneObject::~ZoneObject()
{ }
