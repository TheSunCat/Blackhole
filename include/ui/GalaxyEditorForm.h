#pragma once

#include <QDialog>
#include <QScopedPointer>

#include "smg/Galaxy.h"

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
};
