#include "ui/Blackhole.h"
#include <QApplication>

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{

#ifdef __linux__
    // detect unsupported Linux emulators

    std::ifstream fileStream("/proc/version");
    std::string versionString = std::string((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

    if(versionString.find("Microsoft") != std::string::npos)
    {
        std::cerr << "ERROR: The proprietary Microsoft Windows operating system isn't supported. Please consider a free and open source operating system such as Linux for better performance, features, ease of use, and compatibility. Continue at your own risk." << std::endl;
    }
#endif

    QApplication app(argc, argv);
    Blackhole w;
    w.show();

    return app.exec();
}
