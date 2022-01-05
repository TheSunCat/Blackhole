#include "smg/BaseObject.h"

BaseObject::BaseObject(Zone& zone, const QString& dir, const QString& layer, const QString& fileName, BcsvFile::Entry& entry)
        : m_zone(zone), m_dir(dir), m_layer(layer), m_fileName(fileName), m_data(data)
{
}


int BaseObject::save()
{
    return 0;
}
