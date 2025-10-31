#ifndef DEVICE_H
#define DEVICE_H

#include "deviceinterface.h"
#include <QObject>

class Device : public QObject {
    Q_OBJECT

public:
    explicit Device(QObject *parent = nullptr);
    ~Device() = default;

    void setInterface(DeviceInterface *iface);
    void begin();
    void end();

    QJsonObject requestMixerData();
    bool sendWiFiCretendials(QString& ssid, QString& password);

    bool setFaderPosition(quint16 channel, qint8 value);
    bool toggleMute(quint16 channel);

private:
    DeviceInterface *iface = nullptr;

signals:
    void connected();
    void liveDataReady(QByteArray data);
    void disconnected(DeviceInterface::DisconnectReason reason,
                      const QString& error = "");
};

#endif
