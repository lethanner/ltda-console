#include "device.h"

Device::Device(QObject *parent)
    : QObject{ parent } {

    tcp = new QTcpSocket(this);
    udp = new QUdpSocket(this);

    serial = new QSerialPort(this);
    uartBuffer = new QByteArray();
    uartBuffer->reserve(1024);

    responseWait = new QEventLoop();
    connect(this, &Device::responseReady, this, [this](QByteArray data) {
        qDebug() << "Response ready.";
        responseData = data;
        responseWait->quit();
    });

    pingTicker = new QTimer(this);
    pingTicker->setInterval(10000);
    timeoutTicker = new QTimer(this);
    timeoutTicker->setInterval(timeout);
    timeoutTicker->setSingleShot(true);
    connect(pingTicker, &QTimer::timeout, this, &Device::ping);
    connect(timeoutTicker, &QTimer::timeout, this, [this]() {
        responseWait->quit();
        timeoutFlag = true;
    });

    connect(udp, &QUdpSocket::readyRead, this, &Device::onUdpReadyRead);
    connect(tcp, &QTcpSocket::connected, this, &Device::onTcpConnected);
    connect(tcp, &QTcpSocket::readyRead, this, &Device::onTcpReadyRead);
    connect(tcp, &QTcpSocket::errorOccurred, this, &Device::onTcpError);
    connect(serial, &QSerialPort::readyRead, this, &Device::onSerialReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &Device::onSerialError);

    connect(this, &Device::disconnected, this, [this](DisconnectReason reason,
                                                      const QString& error) {
        if (!error.isEmpty())
            qWarning() << error;

        tcp->close();
        pingTicker->stop();
        //uartBuffer->clear();
        if (serial->isOpen())
            serial->close();

        activeInterface = ConnectionInterface::NONE;
    });
}

void Device::initDevice() {
    static const QByteArray infoRequest = "{\"cmd\":\"info\"}";
    qDebug() << "Requesting device information...";

    QJsonObject dev = commandTxRx(infoRequest);
    if (dev.isEmpty())
        return;

    if (dev["i"].toString() != signature) {
        qWarning() << "Nice try. Disconnecting";
        emit disconnected(InvalidDevice);
        return;
    }

    qInfo() << dev["v"].toString();
    livePacketLength = dev["lsize"].toInt();
    qInfo() << "Live packet length:" << livePacketLength;

    if (activeInterface == LAN) {
        QHostAddress multicastGroup(dev["mca"].toInteger());
        quint16 multicastPort = dev["mcp"].toInt();
        qInfo() << "Device multicast address:" << multicastGroup;

        udp->bind(QHostAddress::AnyIPv4, multicastPort);
        udp->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
        bool isMCJoined = udp->joinMulticastGroup(multicastGroup);

        if (!isMCJoined)
            emit feedbackConnFailed();
    }
    pingTicker->start();
    emit connected();
}

void Device::connectLAN(const QString &host, quint16 port) {
    activeInterface = LAN;
    qInfo() << QString("Connecting to %1:%2...").arg(host).arg(port);

    tcp->connectToHost(host, port);
}

void Device::connectUART(const QString &port, qint32 baud) {
    activeInterface = UART;
    qInfo() << QString("Opening %1 at %2 baud...").arg(port).arg(baud);
    serial->setBaudRate(baud);
    serial->setDataBits(QSerialPort::Data8);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setParity(QSerialPort::NoParity);
    serial->setPortName(port);

    uartBuffer->clear();
    if (!serial->open(QIODevice::ReadWrite)) {
        qWarning() << "Failed to open" << port;
        return;
    }

    initDevice();
}

void Device::disconnect() {
    qInfo() << "Disconnected";
    emit disconnected(Normal);
}

void Device::onSerialError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError)
        return;

    emit disconnected(Specific, serial->errorString());
    //serial->clearError();
    //serial->clear();
}

QStringList Device::getAvailableComPorts() {
    QStringList ports;
    const auto avail = QSerialPortInfo::availablePorts();

    qDebug() << "Enumerating" << avail.count() << "ports";
    for (const QSerialPortInfo &port : avail) {
        ports << port.portName();
        qDebug() << "Port:" << port.portName()
                 << "Description:" << port.description()
                 << "Manufacturer:" << port.manufacturer();
    }

    return ports;
}

void Device::onTcpError(QAbstractSocket::SocketError error) {
    emit disconnected(Specific, tcp->errorString());
}

void Device::onTcpConnected() {
    qInfo() << "TCP connection established";
    initDevice();
}

QJsonObject Device::commandTxRx(const QByteArray& data, bool wait) {
    responseData.clear();
    pingTicker->start(); // reset ping timer
    timeoutFlag = false;

    switch (activeInterface) {
    case LAN:
        qDebug() << "Sending:" << data;
        tcp->write(data);
        break;
    case UART: {
        qsizetype size = data.size();

        QByteArray toSend;
        toSend.reserve(size + 8);

        toSend.append(signature);
        toSend.append(static_cast<char>(0x01)); // version
        toSend.append(static_cast<char>(0x00)); // reserved
        toSend.append(static_cast<char>((size >> 8) & 0xFF));
        toSend.append(static_cast<char>((size) & 0xFF));
        toSend.append(data);

        qDebug() << "Sending:" << toSend;
        serial->write(toSend);
        break;
    }
    default:
        return {};
    }

    /* ***** RECEIVE ***** */
    if (wait) {
        timeoutTicker->start();
        responseWait->exec();
        timeoutTicker->stop();

        if (responseData.isEmpty()) {
            if (timeoutFlag) {
                qWarning() << "Response timed out.";
                emit disconnected(Timeout);
            } else {
                qWarning() << "Unknown error.";
            }
            return {};
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &error);

        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parsing failed:" << error.errorString();
            return {};
        }

        return doc.object();
    } else
        return {};
}

void Device::onSerialReadyRead() {
    QByteArray recvPart = serial->readAll();
    uartBuffer->append(recvPart);

    qsizetype recvLen = uartBuffer->size();
    if (recvLen > 7) { // enough for determining packet information
        if (uartBuffer->startsWith(signature)) {
            qDebug() << "UART buffer:" << uartBuffer->toHex(' ');
            // [0-3] - signature
            // [4] - protocol version
            // [5] - bit 7 is "JSON or live" flag, otherwise reserved
            // [6-7] - packet length

            // checking flag: packet is a JSON response
            if (uartBuffer->at(4) == 0x01 && ((uartBuffer->at(5) >> 7) & 0x01)) {
                quint16 reportedLength = uartBuffer->at(6) << 8 | uartBuffer->at(7);
                if (recvLen < reportedLength + 8)
                    return;
                emit responseReady(uartBuffer->mid(8));
            // packet is a live data (it uses fixed length)
            } else if (!((uartBuffer->at(5) >> 7) & 0x01)) {
                if (recvLen < livePacketLength)
                    return;
                emit liveDataReady(uartBuffer->mid(4));
            } else {
                qWarning() << "Invalid response protocol";
            }
        } else {
            qWarning() << "Invalid packet signature";
        }
        uartBuffer->clear();
    }
}

void Device::onTcpReadyRead() {
    QByteArray data = tcp->readAll();
    emit responseReady(data);
}

void Device::onUdpReadyRead() {
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

void Device::ping()
{
    if (activeInterface == UART) {
        commandTxRx("", false); // zero-length packet used as ping through UART
    } else if (activeInterface == LAN) {
        static QByteArray command = "{\"cmd\":\"ping\"}";
        commandTxRx(command);
    }
}

/* *********************************************************** */

QJsonObject Device::requestMixerData() {
    static QByteArray command = "{\"cmd\":\"list\"}";
    return commandTxRx(command);
}

bool Device::setFaderPosition(quint16 channel, qint8 value) {
    QByteArray command = QString("{\"cmd\":\"fader\",\"ch\":%1,\"val\":%2}")
                                .arg(channel).arg(value).toUtf8();
    return (commandTxRx(command)["status"].toString() == "ok");
}

bool Device::toggleMute(quint16 channel) {
    QByteArray command = QString("{\"cmd\":\"mute\",\"ch\":%1}")
                                .arg(channel).toUtf8();
    return (commandTxRx(command)["status"].toString() == "ok");
}

bool Device::sendWiFiCretendials(QString& ssid, QString& password) {
    QByteArray command = QString("{\"cmd\":\"wifi-sta\",\"ssid\":\"%1\",\"pass\":\"%2\"}")
                                .arg(ssid, password).toUtf8();
    return (commandTxRx(command)["status"].toString() == "ok");
}
