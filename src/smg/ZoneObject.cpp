#include "smg/ZoneObject.h"

#include <QStringList>

ZoneObject::ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : BaseObject(zone, dir, layer, fileName, entry)
{
    // TODO why does Whitehole flip rotation Z and X
    float temp = m_rot.x;
    m_rot.x = m_rot.z;
    m_rot.z = temp;
}

ZoneObject::ZoneObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, const glm::vec3& pos)
        : BaseObject(zone, dir, layer, fileName, pos)
{

}

int ZoneObject::save()
{
    m_data.insert("name", m_name);

    m_data.insert("pos_x", m_pos.x);
    m_data.insert("pos_y", m_pos.y);
    m_data.insert("pos_z", m_pos.z);

    // TODO shouldn't we also flip Z and X here?
    m_data.insert("dir_x", m_rot.x);
    m_data.insert("dir_y", m_rot.y);
    m_data.insert("dir_z", m_rot.z);

    return 0;
}

ZoneObject::~ZoneObject()
{ }
