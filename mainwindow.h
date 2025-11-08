#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
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
#include <QMessageBox>
#include <QCursor>
#include "device.h"
#include "mixerwidget.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
private:
    QWidget* central;
    QVBoxLayout* layout;
    QSplitter* sidePanelSplitter;

    QMenuBar* menuBar;

    QWidget* sidebarWidget;
    QVBoxLayout* sidebar;
    QLabel* logo;

    QWidget* mainWidget;
    QVBoxLayout* mainLayout;

    MixerWidget* mixer;
    QScrollArea* mixerScrollArea;

    QAction *connectAction;
    QAction *disconnectAction;
    QAction *wifiConfAction;

    Device* ltda;
private slots:
    void openWiFiConfigurator();
    void openConnectionWindow();
    void disconnectDevice();

    void connected();
    void disconnected(DeviceInterface::DisconnectReason reason, const QString& error);
};
#endif  // MAINWINDOW_H
