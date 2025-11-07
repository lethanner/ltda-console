#include "mainwindow.h"
#include "mixerchannel.h"
#include "connectiondialog.h"
#include "wifidialog.h"
#include <QPixmap>
#include <QSignalBlocker>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {

    setWindowTitle("LTDA Console");
    resize(1000, 600);

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

    mixerWidget = new QWidget();
    mixer = new QHBoxLayout(mixerWidget);
    mixer->setContentsMargins(8, 8, 8, 8);
    mixer->setSpacing(4);
    mixerScrollArea = new QScrollArea();
    mixerScrollArea->setWidgetResizable(true);
    mixerScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mixerScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mixerScrollArea->setWidget(mixerWidget);
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

    ltda = new Device(this);
    connect(ltda, &Device::connected, this, &MainWindow::connected);
    connect(ltda, &Device::disconnected, this, &MainWindow::disconnected);
    connect(ltda, &Device::liveDataReady, this, &MainWindow::processLiveData);
}

MainWindow::~MainWindow() {
}


void MainWindow::connected() {
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    wifiConfAction->setEnabled(true);

    loadChannels();
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

    // remove all channels from mixer
    QLayoutItem *item;
    while ((item = mixer->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void MainWindow::processLiveData(QByteArray data)
{
    int pos = 2;
    for (int i = 0; i < mixer->count() - 1; ++i) {
        // check channel signature
        if (data[pos++] != 'C' || data[pos++] != 'H') {
            qWarning() << "Malformed packet received!";
            break;
        }

        QLayoutItem *item = mixer->itemAt(i);
        if (item && item->widget()) {
            MixerChannel *channel = qobject_cast<MixerChannel*>(item->widget());
            int8_t faderPos = data[pos++], levelL = data[pos++],
                    levelR = data[pos++], balance = data[pos++];
            bool mute = static_cast<bool>(data[pos++] & 0x01);
            channel->synchronize(faderPos, balance, mute, levelL, levelR);
        }
    }
}

void MainWindow::loadChannels()
{
    QJsonObject data = ltda->requestMixerData();
    if (data.isEmpty()) {
        qWarning() << "Failed to load device channels";
        return;
    }

    if (data.contains("channels") && data["channels"].isArray()) {
        QJsonArray channels = data["channels"].toArray();

        quint16 index = 0;
        for (const auto& channel : channels) {
            if (channel.isObject()) {
                QJsonObject obj = channel.toObject();
                qDebug() << "Appending channel: " << obj["n"].toString();

                QColor color;
                switch (obj["t"].toInt()) {
                case 0:
                    color.setRgb(50, 150, 50);
                    break;
                case 1:
                    color.setRgb(200, 0, 0);
                    break;
                case 2:
                    color.setRgb(170, 170, 0);
                    break;
                case 3:
                    color.setRgb(0, 0, 0);
                    break;
                }

                MixerChannel* newChannel = new MixerChannel(obj["n"].toString(),
                                                            color, obj["st"].toBool(),
                                                            index, mixerWidget);
                mixer->addWidget(newChannel);
                connect(newChannel, &MixerChannel::faderMoved, this, [this, newChannel](const quint16 number, int8_t value){
                    QSignalBlocker blocker(newChannel);
                    ltda->setFaderPosition(number, value);
                });
                connect(newChannel, &MixerChannel::muteClicked, this, [this](const quint16 number){
                    ltda->toggleMute(number);
                });
            }
            index++;
        }
        mixer->addStretch();
    }
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
