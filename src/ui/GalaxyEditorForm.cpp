#include "ui/GalaxyEditorForm.h"
#include "ui_GalaxyEditorForm.h"

#include <ui/Blackhole.h>

GalaxyEditorForm::GalaxyEditorForm(QWidget *parent, const QString& galaxyName) :
    QDialog(parent), m_ui(new Ui::GalaxyEditorForm), m_galaxy(galaxyName)
{
    m_ui->setupUi(this);
    setWindowTitle(QString::fromStdString(blackholeName) + " - Editing " + m_galaxy.m_name);

    for(const QString& zoneName : m_galaxy.m_zoneList)
    {
        // load the zone
        Zone zone = m_galaxy.openZone(zoneName);
        z_zones.push_back(zone);
    }
}

GalaxyEditorForm::~GalaxyEditorForm()
{

}
