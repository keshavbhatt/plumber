#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("Plumber");
    QApplication::setOrganizationName("org.keshavnrj.ubuntu");
    QApplication::setApplicationVersion("1");
    MainWindow w;
    w.show();

    return a.exec();
}
