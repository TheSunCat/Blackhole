#include "ui/Blackhole.h"
#include "ui_Blackhole.h"

#include "filetypes/Bcsv.h"
#include "ui/AboutForm.h"

#include <QFileDialog>
#include <QMessageBox>


Blackhole::Blackhole(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::Blackhole)
{
    m_ui->setupUi(this);

    connect(m_ui->btnSelectGameDir, &QPushButton::clicked, this, &Blackhole::btnSelectGameDirPressed);
    connect(m_ui->btnSettings, &QPushButton::clicked, this, &Blackhole::btnSettingsPressed);
    connect(m_ui->btnAbout, &QPushButton::clicked, this, &Blackhole::btnAboutPressed);
}

Blackhole::~Blackhole()
{

}

void Blackhole::btnSelectGameDirPressed()
{
    QString gameDirPath = QFileDialog::getExistingDirectory(this, "Select game directory");
    if(gameDirPath.isEmpty())
        return;

    m_gameDir = QDir(gameDirPath);

    openGameDir();
}

void Blackhole::btnAboutPressed()
{
    AboutForm *about = new AboutForm(this);
    about->setModal(false);
    about->show();
}

void Blackhole::btnSettingsPressed()
{

}

void Blackhole::openGameDir()
{
    QDir stageData = m_gameDir;
    stageData.cd("StageData");

    QStringList galaxies = stageData.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    if(galaxies.isEmpty())
        return; // game doesn't have galaxies to open

    // detect game type
    // TODO this will break if there is an empty folder in StageData
    QFileInfo firstFile = stageData.path() + '/' + galaxies[0] + ".arc";
    if(firstFile.exists())
        m_gameType = 1;
    else
    {
        QFileInfo firstMapArc = stageData.path() + '/' + galaxies[0] + '/' + galaxies[0] + "Map.arc";
        if(firstMapArc.exists() && firstMapArc.isFile())
            m_gameType = 2;
        else
            m_gameType = 0;
    }

    for(QString galaxy : galaxies)
    {
        Bcsv::addHash(galaxy);

        QFileInfo galaxyScenario = stageData.path() + '/' + galaxy + '/' + galaxy + "Scenario.arc";
        if(!galaxyScenario.exists())
            continue;

        m_ui->galaxyListView->addItem(galaxy);
    }

}
