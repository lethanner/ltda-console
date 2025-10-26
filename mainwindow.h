#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "device.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
private:
  QWidget* central;
    QHBoxLayout* layout;
  QSplitter* sidePanelSplitter;

    QMenuBar* menuBar;

    QWidget* sidebarWidget;
    QVBoxLayout* sidebar;
    QLabel* logo;
    QPushButton* btnConnect;

    QWidget* mainWidget;
    QVBoxLayout* mainLayout;
    QWidget* mixerWidget;
    QHBoxLayout* mixer;
    QScrollArea* mixerScrollArea;

    Device* ltda;

private slots:
    void processLiveData(QByteArray data);
    void loadChannels();

    void openWiFiConfigurator();
    void openConnectionWindow();
};
#endif  // MAINWINDOW_H
