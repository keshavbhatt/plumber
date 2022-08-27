#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/icons/app/icon-256.png"));

  QApplication::setApplicationName("Plumber");
  QApplication::setOrganizationName("org.keshavnrj.ubuntu");
  QApplication::setApplicationVersion(VERSIONSTR);
  MainWindow w;
  w.show();

  return a.exec();
}
