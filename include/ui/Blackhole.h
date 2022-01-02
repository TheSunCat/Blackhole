#pragma once

#include <QMainWindow>
#include <QScopedPointer>
#include <QDir>
#include <string>

class QListWidgetItem;

static std::string blackholeName = "Blackhole v0.1";

namespace Ui {
class Blackhole;
}

class Blackhole : public QMainWindow
{
    Q_OBJECT

public:
    explicit Blackhole(QWidget *parent = nullptr);
    ~Blackhole() override;

    QDir m_gameDir;
    int m_gameType = 0;

private:
    QScopedPointer<Ui::Blackhole> m_ui;

    void btnSelectGameDirPressed();
    void btnSettingsPressed();
    void btnAboutPressed();
    void galaxyListDoubleClicked(QListWidgetItem* item);

    void openGameDir();
};
