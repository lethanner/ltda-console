#include "mainwindow.h"

#include <QApplication>
#include <QPalette>
#include <QIcon>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
    a.setStyle("Fusion");

  QPalette dark;
    dark.setColor(QPalette::Window, QColor(18,18,18));
  dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base, QColor(30,30,30));
  dark.setColor(QPalette::AlternateBase, QColor(45,45,45));
    dark.setColor(QPalette::ToolTipBase, Qt::white);
  dark.setColor(QPalette::ToolTipText, Qt::white);
    dark.setColor(QPalette::Text, Qt::white);
  dark.setColor(QPalette::Button, QColor(45,45,45));
    dark.setColor(QPalette::ButtonText, Qt::white);
  a.setPalette(dark);

  MainWindow w;
  w.setWindowIcon(QIcon(":/img/icon.ico"));
  w.show();
  return a.exec();
}
