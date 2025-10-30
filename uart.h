#ifndef UART_H
#define UART_H

#include "deviceinterface.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class UART : public DeviceInterface
{
    Q_OBJECT

public:
    UART();

    bool _connect() override;
    //void _disconnect() override;

    void setPort(const QString& _port, qint32 _baud)
        { port = _port, baud = _baud; }

    void completeInitialization(QJsonObject& devInfo) override;

    QStringList getAvailableComPorts();
    void ping() override;

private:
    QSerialPort *serial;
    QByteArray *uartBuffer;
    quint16 livePacketLength = 0;

    QString port;
    qint32 baud;

    void send(const QByteArray& data) override;
    //QJsonObject receiveBlocking() override;

private slots:
    void onSerialReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);
};

#endif // UART_H
