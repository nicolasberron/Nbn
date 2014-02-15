#ifdef _WIN32
#include <Windows.h>
#endif

#include <QApplication>

#include "NBNDevLauncherMainWindow.h"

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    NBNDevLauncherMainWindow w;
    w.show();

    return a.exec();
}

