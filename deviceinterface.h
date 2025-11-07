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
    virtual void _disconnect(DisconnectReason reason, const QString& error = "");

    virtual void send(const QByteArray& data) = 0;
    QJsonObject receiveBlocking();

    virtual void ping() = 0;
    //virtual void liveDataReady(QByteArray data) = 0;

    virtual void completeInitialization(QJsonObject& devInfo);

private slots:

protected:
    const QByteArray signature = "LTDA";

    void responseReady(QByteArray response, bool forceUnblock = false);

signals:
    void disconnected(DisconnectReason reason, const QString& error = "");
    void liveDataReady(QByteArray data);

private:
    QByteArray responseData;
    QEventLoop *responseWait;
    QTimer *timeoutTicker;
    QTimer *pingTicker;

    bool timeoutFlag;
    const int timeout = 1000;

    bool forceUnblock = false;
};

#endif // DEVICEINTERFACE_H
