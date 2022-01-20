#include "rendering/ObjectRenderer.h"

#include "Util.h"

ObjectRenderer::ObjectRenderer(BaseObject* obj)
        : m_object(obj), m_translation(obj->m_pos),
          m_rotation(obj->m_rot), m_scale(obj->m_scl),
          m_modelName(obj->m_name)
{
    QString filePath = absolutePath("ObjectData/" + m_modelName + ".arc");

    QFileInfo fileInfo = QFileInfo(filePath);
    if(!fileInfo.exists())
        return; // TODO fallback to cube

    m_rarc = RarcFile(filePath);
    if(m_rarc.fileExists('/' + m_modelName + '/' + m_modelName + ".bdl"))
        m_model = BmdFile(m_rarc.openFile('/' + m_modelName + '/' + m_modelName + ".bdl"));
    else if(m_rarc.fileExists('/' + m_modelName + '/' + m_modelName + ".bmd"))
        m_model = BmdFile(m_rarc.openFile('/' + m_modelName + '/' + m_modelName + ".bmd"));
}
