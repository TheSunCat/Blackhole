#pragma once

#include <QDialog>
#include <QScopedPointer>

namespace Ui
{
class GalaxyEditorForm;
}

class GalaxyEditorForm : public QDialog
{
    Q_OBJECT

public:
    explicit GalaxyEditorForm(QWidget *parent, QString galaxyName);
    ~GalaxyEditorForm() override;


private:
    QScopedPointer<Ui::GalaxyEditorForm> m_ui;
};
