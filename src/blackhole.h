#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui {
class Blackhole;
}

class Blackhole : public QMainWindow
{
    Q_OBJECT

public:
    explicit Blackhole(QWidget *parent = nullptr);
    ~Blackhole() override;

private:
    QScopedPointer<Ui::Blackhole> m_ui;
};

#endif // BLACKHOLE_H
