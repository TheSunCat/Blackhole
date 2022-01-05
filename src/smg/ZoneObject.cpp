#include "smg/ZoneObject.h"

#include <QStringList>

ZoneObject::ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : BaseObject(zone, dir, layer, fileName, entry)
{
    m_name = m_data["name"];

    m_pos = glm::vec3(
        m_data["pos_x"],
        m_data["pos_y"],
        m_data["pos_z"]
    );

    m_rot = glm::vec3(
        m_data["dir_z"],
        m_data["dir_y"],
        m_data["dir_x"]
    );

    // TODO why scl = 1?
    m_scl = glm::vec3(1);
}

// TODO *new jank?
ZoneObject::ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos)
        : BaseObject(zone, dir, layer, fileName, *new BcsvFile::Entry())
{
    m_pos = pos;
    m_rot = glm::vec3(0);
    m_scl = glm::vec3(1);

    m_data.insert("name", m_name.toStdString());
}
