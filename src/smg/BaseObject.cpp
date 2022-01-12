#include "smg/BaseObject.h"

BaseObject::BaseObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : m_zone(zone), m_dir(dir), m_layer(layer), m_fileName(fileName), m_data(entry)
{
    m_name = m_data.getstr("name");

    m_pos = glm::vec3(
        m_data.getf("pos_x"),
        m_data.getf("pos_y"),
        m_data.getf("pos_z")
    );

    m_rot = glm::vec3(
        m_data.getf("dir_x"),
        m_data.getf("dir_y"),
        m_data.getf("dir_z")
    );

    m_scl = glm::vec3(1);
}

// todo *new jank?
BaseObject::BaseObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, const glm::vec3& pos)
        : m_zone(zone), m_dir(dir), m_layer(layer), m_fileName(fileName), m_data(*new BcsvFile::Entry())
{
    m_pos = pos;
    m_rot = glm::vec3(0);
    m_scl = glm::vec3(1);

    m_data.insert("name", m_name);
    m_data.insert("l_id", uint32_t(0));

    m_data.insert("pos_x", m_pos.x);
    m_data.insert("pos_y", m_pos.y);
    m_data.insert("pos_z", m_pos.z);

    m_data.insert("dir_x", m_rot.x);
    m_data.insert("dir_y", m_rot.y);
    m_data.insert("dir_z", m_rot.z);
}


int BaseObject::save()
{
    return 0;
}
