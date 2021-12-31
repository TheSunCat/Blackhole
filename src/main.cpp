#include "blackhole.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Blackhole w;
    w.show();

    return app.exec();
}

