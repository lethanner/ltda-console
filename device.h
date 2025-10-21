#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class Device : public QObject {
  Q_OBJECT

public:
  explicit Device(QObject *parent = nullptr);

    enum DisconnectReason {
        Normal, Timeout, InvalidDevice, Specific
    };

    void connectTo(const QString &host, quint16 port);
    void disconnect();

    void requestMixerData();
    void setFaderPosition(quint16 channel, qint8 value);
    void toggleMute(quint16 channel);
    void sendWiFiCretendials(QString& ssid, QString& password);

    QStringList getAvailableComPorts();

  private:
    const QByteArray signature = "LTDA";

    void sendTCP(QByteArray& data);
    QTcpSocket *tcp;
    QUdpSocket *udp;

    QTimer *keepAliveTicker;
    QTimer *timeoutTicker;
    int tcpTimeout = 1000;
    int latency = 0;
    bool throttleFlag = false;

    bool commPending = false;

signals:
    void connected();
    void disconnected(DisconnectReason reason,
                      QAbstractSocket::SocketError *error = nullptr);
    void feedbackConnFailed();

    void udpDataReady(QByteArray& data);
    void responseReady(QJsonObject& data);
    void responseStatus(QString status, QString why);

private slots:
    void onTcpConnected();
    void onTcpReadyRead();
    void onUdpReadyRead();
    void onTcpError(QAbstractSocket::SocketError error);
    void onTimeout();

    void sendKeepAlive();
};

#endif  // DEVICE_H
