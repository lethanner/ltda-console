#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "deviceinterface.h"

class Simulator : public DeviceInterface
{
    Q_OBJECT

public:
    Simulator();

    bool _connect() override;
    void _disconnect(DisconnectReason reason, const QString& error) override;
    void completeInitialization(QJsonObject& devInfo) override;
    void send(const QByteArray& data) override;
    void ping() override;
};

#endif // SIMULATOR_H
