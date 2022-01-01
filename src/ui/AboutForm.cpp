#include "ui/AboutForm.h"
#include "ui_AboutForm.h"

#include <ui/Blackhole.h>

AboutForm::AboutForm(QWidget *parent) :
    QDialog(parent), m_ui(new Ui::AboutForm)
{
    m_ui->setupUi(this);
    m_ui->lblBlackholeName->setText(QString::fromStdString(blackholeName));
}

AboutForm::~AboutForm()
{

}
