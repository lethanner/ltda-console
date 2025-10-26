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
#include <QEventLoop>

class Device : public QObject {
  Q_OBJECT

public:
  explicit Device(QObject *parent = nullptr);

    enum DisconnectReason {
        Normal, Timeout, InvalidDevice, Specific
    };

    enum ConnectionInterface {
        NONE, LAN, UART
    };

    void connectLAN(const QString &host, quint16 port);
    void connectUART(const QString &port, qint32 baud);
    void disconnect();

    QJsonObject requestMixerData();
    bool setFaderPosition(quint16 channel, qint8 value);
    bool toggleMute(quint16 channel);
    bool sendWiFiCretendials(QString& ssid, QString& password);

    QStringList getAvailableComPorts();

  private:
    const QByteArray signature = "LTDA";
    const uint timeout = 1000;

    QJsonObject commandTxRx(const QByteArray& data, bool wait = true);
    void initDevice();

    QSerialPort *serial;
    QByteArray *uartBuffer;
    quint16 livePacketLength = 0;

    QTcpSocket *tcp;
    QUdpSocket *udp;

    QTimer *pingTicker;
    QTimer *timeoutTicker;
    bool timeoutFlag = false;
    //int latency = 0;

    ConnectionInterface activeInterface = ConnectionInterface::NONE;
    QByteArray responseData;
    QEventLoop *responseWait;

signals:
    void connected();
    void disconnected(DisconnectReason reason, const QString& error = "");
    void feedbackConnFailed();

    void liveDataReady(QByteArray data);
    void responseReady(QByteArray data);
    //void responseStatus(QString status, QString why);

private slots:
    void onTcpConnected();
    void onTcpReadyRead();
    void onUdpReadyRead();
    void onTcpError(QAbstractSocket::SocketError error);
    void onSerialReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

    void ping();
};

#endif  // DEVICE_H
