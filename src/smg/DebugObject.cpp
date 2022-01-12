#include "smg/DebugObject.h"

#include "ui/Blackhole.h"

DebugObject::DebugObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : BaseObject(zone, dir, layer, fileName, entry)
{
    m_scl = glm::vec3(
        m_data.getf("scale_x"),
        m_data.getf("scale_y"),
        m_data.getf("scale_z")
    );
}

DebugObject::DebugObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos)
        : BaseObject(zone, dir, layer, fileName, pos)
{
    m_data.insert("scale_x", m_scl.x);
    m_data.insert("scale_y", m_scl.y);
    m_data.insert("scale_z", m_scl.z);

    m_data.insert("PosName", "undefined");
    m_data.insert("Obj_ID", uint16_t(-1));

    if(g_gameType == 1)
        m_data.insert("ChildObjId", uint16_t(-1));
}

int DebugObject::save()
{
    m_data.insert("name", m_name);

    m_data.insert("pos_x", m_pos.x);
    m_data.insert("pos_y", m_pos.y);
    m_data.insert("pos_z", m_pos.z);

    m_data.insert("dir_x", m_rot.x);
    m_data.insert("dir_y", m_rot.y);
    m_data.insert("dir_z", m_rot.z);

    m_data.insert("scale_x", m_scl.x);
    m_data.insert("scale_y", m_scl.y);
    m_data.insert("scale_z", m_scl.z);

    // TODO shouldn't we save the other data, too?

    return 0;
}

DebugObject::~DebugObject()
{ }
