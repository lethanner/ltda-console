#include "uart.h"

UART::UART() {
    serial = new QSerialPort();
    uartBuffer = new QByteArray();
    uartBuffer->reserve(1024);

    connect(serial, &QSerialPort::readyRead, this, &UART::onSerialReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &UART::onSerialError);
}

bool UART::_connect() {
    qInfo() << QString("Opening %1 at %2 baud...").arg(port).arg(baud);
    serial->setBaudRate(baud);
    serial->setDataBits(QSerialPort::Data8);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setParity(QSerialPort::NoParity);
    serial->setPortName(port);

    uartBuffer->clear();
    bool result = serial->open(QIODevice::ReadWrite);
    if (!result)
        qWarning() << "Failed to open" << port;

    return result;
}

void UART::_disconnect(DisconnectReason reason, const QString& error) {
    qDebug() << "Closing UART";

    if (serial->isOpen())
        serial->close();

    DeviceInterface::_disconnect(reason, error);
}

void UART::completeInitialization(QJsonObject& devInfo) {
    livePacketLength = devInfo["lsize"].toInt();
    qInfo() << "Live packet length:" << livePacketLength;

    DeviceInterface::completeInitialization(devInfo);
}

void UART::send(const QByteArray& data) {
    DeviceInterface::send(data);

    qsizetype size = data.size();

    QByteArray toSend;
    toSend.reserve(size + 8);
    toSend.append(signature);
    toSend.append(static_cast<char>(0x01)); // version
    toSend.append(static_cast<char>(0x00)); // reserved
    toSend.append(static_cast<char>((size >> 8) & 0xFF));
    toSend.append(static_cast<char>((size) & 0xFF));
    toSend.append(data);

    serial->write(toSend);
}

void UART::ping() {
    send("");
}

void UART::onSerialReadyRead() {
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
                responseReady(uartBuffer->mid(8));
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

void UART::onSerialError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError)
        return;

    _disconnect(Specific, serial->errorString());
    //serial->clearError();
    //serial->clear();
}

QStringList UART::getAvailableComPorts() {
    QStringList ports;
    const auto avail = QSerialPortInfo::availablePorts();

    qDebug() << "Enumerating" << avail.count() << "ports";
    for (const QSerialPortInfo &port : avail) {
        ports.append(port.portName());
        qDebug() << "Port:" << port.portName()
                 << "Description:" << port.description()
                 << "Manufacturer:" << port.manufacturer();
    }

    return ports;
}
