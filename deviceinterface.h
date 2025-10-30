#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QEventLoop>
#include <QTimer>

class DeviceInterface : public QObject
{
    Q_OBJECT

public:
    explicit DeviceInterface(QObject *parent = nullptr);
    virtual ~DeviceInterface() = default;

    enum DisconnectReason {
        Normal, Timeout, InvalidDevice, Specific
    };

    virtual bool _connect() = 0;
    //virtual void _disconnect() = 0;

    virtual void send(const QByteArray& data) = 0;
    virtual void ping() = 0;
    QJsonObject receiveBlocking();

    virtual void completeInitialization(QJsonObject& devInfo);

protected:
    const QByteArray signature = "LTDA";

    void responseReady(QByteArray response);
    //void liveDataReady(QByteArray data);

signals:
    void liveDataReady(QByteArray data);
    void disconnected(DisconnectReason reason,
                      const QString& error = "");

private:
    QByteArray responseData;
    QEventLoop *responseWait;
    QTimer *timeoutTicker;
    QTimer *pingTicker;

    bool timeoutFlag;
    const int timeout = 1000;
};

#endif // DEVICEINTERFACE_H
