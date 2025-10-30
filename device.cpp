#include "device.h"

Device::Device(QObject *parent) : QObject{ parent } {

}

void Device::begin(DeviceInterface *_iface) {
    iface = _iface;
    if (iface->_connect()) {
        static const QByteArray infoRequest = "{\"cmd\":\"info\"}";
        qDebug() << "Requesting device information...";

        iface->send(infoRequest);
        QJsonObject dev = iface->receiveBlocking();

        if (dev.isEmpty())
            return;

        if (dev["i"].toString() != "LTDA") {
            qWarning() << "Nice try. Disconnecting";
            end(DeviceInterface::InvalidDevice);
            return;
        }
        qInfo() << dev["v"].toString();

        iface->completeInitialization(dev);

        emit deviceReady();
    }
}

void Device::end(DeviceInterface::DisconnectReason reason) {

    emit iface->disconnected(reason);
    if (reason == DeviceInterface::Normal)
        qInfo() << "Disconnected by user request";
}

QJsonObject Device::requestMixerData() {
    if (!iface) return {};

    static QByteArray command = "{\"cmd\":\"list\"}";

    iface->send(command);
    return iface->receiveBlocking();
}

bool Device::setFaderPosition(quint16 channel, qint8 value) {
    if (!iface) return false;

    QByteArray command = QString("{\"cmd\":\"fader\",\"ch\":%1,\"val\":%2}")
                            .arg(channel).arg(value).toUtf8();

    iface->send(command);
    return (iface->receiveBlocking()["status"].toString() == "ok");
}

bool Device::toggleMute(quint16 channel) {
    if (!iface) return false;

    QByteArray command = QString("{\"cmd\":\"mute\",\"ch\":%1}")
                            .arg(channel).toUtf8();

    iface->send(command);
    return (iface->receiveBlocking()["status"].toString() == "ok");
}

bool Device::sendWiFiCretendials(QString& ssid, QString& password) {
    if (!iface) return false;

    QByteArray command = QString("{\"cmd\":\"wifi-sta\",\"ssid\":\"%1\",\"pass\":\"%2\"}")
                            .arg(ssid, password).toUtf8();

    iface->send(command);
    return (iface->receiveBlocking()["status"].toString() == "ok");
}
