#include "device.h"

Device::Device(QObject *parent)
  : QObject{ parent } {
    keepAliveTicker = new QTimer(this);
    timeoutTicker = new QTimer(this);

    tcp = new QTcpSocket(this);
    udp = new QUdpSocket(this);

    connect(udp, &QUdpSocket::readyRead, this, &Device::onUdpReadyRead);
    connect(tcp, &QTcpSocket::connected, this, &Device::onTcpConnected);
    connect(tcp, &QTcpSocket::readyRead, this, &Device::onTcpReadyRead);
    connect(tcp, &QTcpSocket::errorOccurred, this, &Device::onTcpError);

    connect(keepAliveTicker, &QTimer::timeout, this, &Device::sendKeepAlive);
    connect(timeoutTicker, &QTimer::timeout, this, &Device::onTimeout);
    keepAliveTicker->setInterval(10000);
    timeoutTicker->setInterval(tcpTimeout);
    timeoutTicker->setSingleShot(true);

    connect(this, &Device::disconnected, this, [this](DisconnectReason reason,
                                             QAbstractSocket::SocketError *error) {
        timeoutTicker->stop();
        keepAliveTicker->stop();
        if (reason != Normal) tcp->abort();
        if (error) qWarning() << *error;
    });
    connect(this, &Device::responseStatus, this, [this](QString status, QString why) {
        if (status != "ok")
            qWarning() << "Device returned error:" << why;
        else {
            qDebug() << "Device response OK.";
            throttleFlag = false;
        }
    });
}

void Device::connectTo(const QString& host, quint16 port) {
    qInfo() << QString("Connecting to %1:%2...").arg(host).arg(port);
    tcp->connectToHost(host, port);
    commPending = true;
}

void Device::disconnect() {
    qInfo() << "Disconnected";
    tcp->close();
    emit disconnected(Normal);
}

void Device::onTcpConnected() {
    qInfo() << "Connection established";

    // check if it really an LTDA device
    static QByteArray infoRequest = "{\"cmd\":\"info\"}";
    sendTCP(infoRequest);
}

void Device::onTcpReadyRead() {
    if (timeoutTicker->isActive()) {
        latency = tcpTimeout - timeoutTicker->remainingTime();
        timeoutTicker->stop();
    }

    QByteArray data = tcp->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parsing failed:" << error.error;
        return;
    }

    QJsonObject root = doc.object();
    // receiving device info packet
    if (commPending) {
        commPending = false;
        if (root["i"].toString("") != signature) {
            qWarning() << "Wrong packet received.";
            emit disconnected(InvalidDevice);
            return;
        }

        qInfo() << root["v"].toString();

        QHostAddress multicastGroup(root["mca"].toInteger());
        quint16 multicastPort = root["mcp"].toInt();
        qInfo() << "Device multicast address:" << multicastGroup;

        udp->bind(QHostAddress::AnyIPv4, multicastPort);
        udp->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
        bool isMCJoined = udp->joinMulticastGroup(multicastGroup);

        if (!isMCJoined)
            emit feedbackConnFailed();

        keepAliveTicker->start();
        emit connected();
    // status messages
    } else if (root.contains("status")) {
        emit responseStatus(root["status"].toString(),
                            root["why"].toString(""));
    // any other received messages
    } else
        emit responseReady(root);
}

void Device::onUdpReadyRead() {
    while (udp->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udp->pendingDatagramSize());
        udp->readDatagram(datagram.data(), datagram.size());

        // check packet signature
        if (datagram.mid(0, 4) == signature) {
            emit udpDataReady(datagram);
        } else {
            qWarning() << "Unknown datagram: " << datagram;
        }
    }
}

void Device::onTcpError(QAbstractSocket::SocketError error) {
    qWarning() << "Connection lost";
    emit disconnected(Specific, &error);
}

void Device::sendTCP(QByteArray& data) {
    tcp->write(data);
    timeoutTicker->start();
    keepAliveTicker->start(); // reset keep alive timer
    qDebug() << "Sending:" << data;
}

void Device::sendKeepAlive()
{
    static QByteArray command = "{\"cmd\":\"ping\"}";
    sendTCP(command);

    qDebug() << "Keep-alive packet sent";
}

void Device::onTimeout() {
    qWarning() << "Response timed out.";
    emit disconnected(Timeout);
}

void Device::requestMixerData() {
    static QByteArray command = "{\"cmd\":\"list\"}";
    sendTCP(command);
}

void Device::setFaderPosition(quint16 channel, qint8 value) {
    QByteArray command = QString("{\"cmd\":\"fader\",\"ch\":%1,\"val\":%2}")
                              .arg(channel).arg(value).toUtf8();

    if (!throttleFlag) {
        throttleFlag = true;
        sendTCP(command);
    }
}

void Device::toggleMute(quint16 channel) {
    QByteArray command = QString("{\"cmd\":\"mute\",\"ch\":%1}")
                                .arg(channel).toUtf8();
    sendTCP(command);
}

void Device::sendWiFiCretendials(QString& ssid, QString& password) {
    QByteArray command = QString("{\"cmd\":\"wifi-sta\",\"ssid\":\"%1\",\"pass\":\"%2\"}")
                                                    .arg(ssid, password).toUtf8();
    sendTCP(command);
}
