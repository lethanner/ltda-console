#include "deviceinterface.h"

DeviceInterface::DeviceInterface(QObject *parent)
    : QObject{ parent } {

    responseWait = new QEventLoop();
    timeoutTicker = new QTimer();
    pingTicker = new QTimer(this);

    pingTicker->setInterval(10000); // temporarily hard-coded
    timeoutTicker->setInterval(timeout);
    timeoutTicker->setSingleShot(true);
    connect(timeoutTicker, &QTimer::timeout, this, [this]() {
        responseWait->quit();
        timeoutFlag = true;
    });
    connect(pingTicker, &QTimer::timeout, this, &DeviceInterface::ping);

    connect(this, &DeviceInterface::disconnected, this, [this](DisconnectReason reason,
                                                               const QString& error) {
        qDebug() << "Disconnecting";
        if (!error.isEmpty())
            qWarning() << error;

        timeoutTicker->stop();
        pingTicker->stop();
    });
}

void DeviceInterface::completeInitialization(QJsonObject& devInfo) {
    pingTicker->start();
}

void DeviceInterface::send(const QByteArray& data) {
    pingTicker->start(); // reset ping timer
    responseData.clear();
    timeoutFlag = false;

    qDebug() << "Sending:" << data;
}

QJsonObject DeviceInterface::receiveBlocking() {
    timeoutTicker->start();
    responseWait->exec();
    timeoutTicker->stop();

    if (responseData.isEmpty()) {
        if (timeoutFlag)
            qWarning() << "Response timed out.";
        else
            qWarning() << "Unknown error.";
        return {};
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parsing failed:" << error.errorString();
        return {};
    }

    return doc.object();
}

void DeviceInterface::responseReady(QByteArray response) {
    responseData = response;
    responseWait->quit();
}
