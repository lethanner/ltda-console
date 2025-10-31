#ifndef LAN_H
#define LAN_H

#include "deviceinterface.h"
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QAbstractSocket>

class LAN : public DeviceInterface
{
    Q_OBJECT

public:
    LAN();

    bool _connect() override;
    void _disconnect(DisconnectReason reason, const QString& error) override;

    void setHost(const QString& _host, quint16 _port)
        { host = _host, port = _port; }

    void completeInitialization(QJsonObject& devInfo) override;

    void ping() override;

private:
    QString host;
    quint16 port;

    QTcpSocket *tcp;
    QUdpSocket *udp;

    void send(const QByteArray& data) override;

private slots:
    void onTcpReadyRead();
    void onUdpReadyRead();
    void onTcpError(QAbstractSocket::SocketError error);
};

#endif // LAN_H
