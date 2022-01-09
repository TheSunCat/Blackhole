#pragma once

#include <QDialog>
#include <QScopedPointer>
#include <QString>
#include <string>

#include "smg/Galaxy.h"
#include "smg/Zone.h"
#include "smg/BaseObject.h"

namespace Ui
{
class GalaxyEditorForm;
}

class GalaxyEditorForm : public QDialog
{
    Q_OBJECT

public:
    explicit GalaxyEditorForm(QWidget *parent, const QString& galaxyName);
    ~GalaxyEditorForm() override;


private:
    QScopedPointer<Ui::GalaxyEditorForm> m_ui;

    Galaxy m_galaxy;
    std::unordered_map<std::string, Zone> m_zones;

    std::vector<BaseObject*> m_objects;
    std::unordered_map<std::string, ZoneObject*> m_zoneObjects;
};
