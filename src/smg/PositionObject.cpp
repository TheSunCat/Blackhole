#include "smg/PositionObject.h"

#include "ui/Blackhole.h"

PositionObject::PositionObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : BaseObject(zone, dir, layer, fileName, entry)
{
}

PositionObject::PositionObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos)
        : BaseObject(zone, dir, layer, fileName, pos)
{
    m_data.insert("PosName", "undefined");
    m_data.insert("Obj_ID", uint16_t(-1));

    if(g_gameType == 1)
        m_data.insert("ChildObjId", uint16_t(-1));
}

int PositionObject::save()
{
    m_data.insert("name", m_name);

    m_data.insert("pos_x", m_pos.x);
    m_data.insert("pos_y", m_pos.y);
    m_data.insert("pos_z", m_pos.z);

    m_data.insert("dir_x", m_rot.x);
    m_data.insert("dir_y", m_rot.y);
    m_data.insert("dir_z", m_rot.z);

    // TODO shouldn't we save the other data, too?

    return 0;
}

PositionObject::~PositionObject()
{ }
