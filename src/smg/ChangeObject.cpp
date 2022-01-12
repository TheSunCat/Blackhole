#include "smg/ChangeObject.h"

#include "ui/Blackhole.h"

ChangeObject::ChangeObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : BaseObject(zone, dir, layer, fileName, entry)
{
    m_scl = glm::vec3(
        m_data.getf("scale_x"),
        m_data.getf("scale_y"),
        m_data.getf("scale_z")
    );
}

ChangeObject::ChangeObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, glm::vec3 pos)
        : BaseObject(zone, dir, layer, fileName, pos)
{
    m_data.insert("SW_APPEAR", uint32_t(-1));
    m_data.insert("SW_DEAD", uint32_t(-1));
    m_data.insert("SW_A", uint32_t(-1));
    m_data.insert("SW_B", uint32_t(-1));

    // TODO what are these field names?
    m_data.insert(0x55CFC442, uint16_t(-1));
    m_data.insert(0xC4F73392, uint16_t(-1));
    m_data.insert("l_id", uint32_t(0));
}

int ChangeObject::save()
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

ChangeObject::~ChangeObject()
{ }
