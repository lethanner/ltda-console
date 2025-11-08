#include "mainwindow.h"
#include "connectiondialog.h"
#include "wifidialog.h"
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {

    setWindowTitle("LTDA Console");
    resize(1000, 600);

    ltda = new Device(this);

    central = new QWidget(this);
    setCentralWidget(central);

    layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    menuBar = new QMenuBar();
    QMenu *fileContext = menuBar->addMenu("File");
    QMenu *deviceContext = menuBar->addMenu("Device");
    connectAction = new QAction("Connect...", this);
    disconnectAction = new QAction("Disconnect", this);
    wifiConfAction = new QAction("Configure Wi-Fi...", this);
    deviceContext->addAction(connectAction);
    deviceContext->addAction(disconnectAction);
    deviceContext->addSeparator();
    deviceContext->addAction(wifiConfAction);
    disconnectAction->setEnabled(false);
    wifiConfAction->setEnabled(false);
    connect(connectAction, &QAction::triggered, this, &MainWindow::openConnectionWindow);
    connect(disconnectAction, &QAction::triggered, this, &MainWindow::disconnectDevice);
    connect(wifiConfAction, &QAction::triggered, this, &MainWindow::openWiFiConfigurator);
    setMenuBar(menuBar);

    // Sidebar widget + layout
    sidebarWidget = new QWidget();
    sidebar = new QVBoxLayout(sidebarWidget);
    sidebar->setContentsMargins(28, 16, 28, 16);
    sidebar->setSpacing(8);

    QPixmap logoImage(":/img/logo.png");
    logo = new QLabel(sidebarWidget);
    logo->setPixmap(logoImage);
    logo->setScaledContents(true);
    sidebar->addWidget(logo, 0, Qt::AlignCenter);
    sidebar->addStretch(0);

    // Main area widget + layout
    // TODO: replace with QStackedWidget
    mainWidget = new QWidget();
    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mixer = new MixerWidget(ltda, this);
    mixerScrollArea = new QScrollArea();
    mixerScrollArea->setWidgetResizable(true);
    mixerScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mixerScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mixerScrollArea->setWidget(mixer);
    mainLayout->addWidget(mixerScrollArea);

    // Splitter
    sidePanelSplitter = new QSplitter(Qt::Horizontal);
    sidePanelSplitter->addWidget(sidebarWidget);
    sidePanelSplitter->addWidget(mainWidget);
    sidePanelSplitter->setStretchFactor(1, 1);
    sidePanelSplitter->setCollapsible(0, true);
    layout->addWidget(sidePanelSplitter);

    // version string
    QLabel *version = new QLabel("v0.1.3-alpha | #MakePrismaTechniciansGreatAgain", this);
    version->setMaximumHeight(30);
    version->setAlignment(Qt::AlignCenter);
    version->setStyleSheet("color:gray;font-size:12px;");
    layout->addWidget(version);

    connect(ltda, &Device::connected, this, &MainWindow::connected);
    connect(ltda, &Device::disconnected, this, &MainWindow::disconnected);
}

MainWindow::~MainWindow() {
}


void MainWindow::connected() {
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    wifiConfAction->setEnabled(true);

    mixer->load();
}

void MainWindow::disconnected(DeviceInterface::DisconnectReason reason, const QString& error) {
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    wifiConfAction->setEnabled(false);

    QString message;
    switch (reason) {
    case DeviceInterface::Normal:
        break;
    case DeviceInterface::InvalidDevice:
        message = "Your address points on an invalid device.";
        break;
    case DeviceInterface::Timeout:
        message = "Connection lost.";
        break;
    case DeviceInterface::Specific:
        message = error;
        break;
    }
    if (!message.isEmpty())
        QMessageBox::critical(this, "LTDA Console", message);

    mixer->clear();
}

void MainWindow::openWiFiConfigurator() {
    WiFiDialog *dialog = new WiFiDialog(this);
    if (dialog->exec() == QDialog::Accepted) {
        ltda->sendWiFiCretendials(WiFiDialog::inputData.ssid,
                                  WiFiDialog::inputData.password);
    }
}

void MainWindow::openConnectionWindow() {
    ConnectionDialog *dialog = new ConnectionDialog(ltda, this);

    if (dialog->exec()) {
        connectAction->setEnabled(false);

        QApplication::setOverrideCursor(Qt::WaitCursor);
        ltda->begin();
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::disconnectDevice() {
    ltda->end();
}
