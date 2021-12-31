#include "blackhole.h"
#include "ui_blackhole.h"

Blackhole::Blackhole(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Blackhole)
{
    m_ui->setupUi(this);
}

Blackhole::~Blackhole() = default;
