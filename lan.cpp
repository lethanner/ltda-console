#include "lan.h"

LAN::LAN() {
    tcp = new QTcpSocket();
    udp = new QUdpSocket();

    connect(udp, &QUdpSocket::readyRead, this, &LAN::onUdpReadyRead);
    connect(tcp, &QTcpSocket::readyRead, this, &LAN::onTcpReadyRead);
    connect(tcp, &QTcpSocket::errorOccurred, this, &LAN::onTcpError);
    connect(this, &DeviceInterface::disconnected, this, [this](DisconnectReason reason,
                                                               const QString& error) {
        if (tcp->isOpen()) {
            if (reason == Timeout || !error.isEmpty())
                tcp->abort();
            else
                tcp->close();
        }

        if (udp->isOpen())
            udp->close();
    });
}

bool LAN::_connect() {
    qInfo() << QString("Connecting to %1:%2...").arg(host).arg(port);

    tcp->connectToHost(host, port);
    bool result = tcp->waitForConnected(5000);

    qInfo() << (result ? "TCP connection established" : "Failed to connect");

    return result;
}

void LAN::send(const QByteArray& data) {
    DeviceInterface::send(data);
    tcp->write(data);
}

void LAN::ping() {
    static QByteArray command = "{\"cmd\":\"ping\"}";
    send(command);
}

void LAN::completeInitialization(QJsonObject& devInfo)
{
    QHostAddress multicastGroup(devInfo["mca"].toInteger());
    quint16 multicastPort = devInfo["mcp"].toInt();
    qInfo() << "Device multicast address:" << multicastGroup;

    udp->bind(QHostAddress::AnyIPv4, multicastPort);
    udp->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
    /*bool isMCJoined = */udp->joinMulticastGroup(multicastGroup);

    DeviceInterface::completeInitialization(devInfo);
}

void LAN::onTcpReadyRead() {
    QByteArray data = tcp->readAll();
    responseReady(data);
}

void LAN::onTcpError(QAbstractSocket::SocketError error) {
    emit disconnected(Specific, tcp->errorString());
}

void LAN::onUdpReadyRead() {
    while (udp->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udp->pendingDatagramSize());
        udp->readDatagram(datagram.data(), datagram.size());

        // check packet signature
        if (datagram.mid(0, 4) == signature) {
            emit liveDataReady(datagram.mid(4));
        } else {
            qWarning() << "Unknown UDP datagram: " << datagram;
        }
    }
}
