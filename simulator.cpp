#include "simulator.h"
#include <QRandomGenerator>

Simulator::Simulator() {}

bool Simulator::_connect() {
    qInfo() << "Running in simulation mode.";
    return true;
}

void Simulator::_disconnect(DisconnectReason reason, const QString& error) {
    DeviceInterface::_disconnect(reason, error);
}


void Simulator::completeInitialization(QJsonObject& devInfo) {
    DeviceInterface::completeInitialization(devInfo);
}

void Simulator::send(const QByteArray& data) {
    DeviceInterface::send(data);

    if (data.isEmpty()) {
        return;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Simulator: failed to parse JSON:" << error.errorString();
        return;
    }

    static const QByteArray ok = "{\"status\":\"ok\"}";
    if (doc["cmd"].toString() == "info") {
        responseReady("{\"i\":\"LTDA\",\"v\":\"Simulin v.9000\"}", true);
    } else if (doc["cmd"].toString() == "list") {
        QJsonObject fakeList;
        QJsonArray fakeChannels;

        for (int i = 0; i < 16; i++) {
            static const QString typeLabel[] = {"In", "AUX", "FX", "Out"};
            quint8 type = static_cast<quint8>(i / 4);
            if (type > 3) type = 3;

            QJsonObject fakeChannel;
            fakeChannel["n"] = QString("Fake %1 %2").arg(typeLabel[type]).arg(i);
            fakeChannel["t"] = type;
            fakeChannel["st"] = false;
            fakeChannels.append(fakeChannel);
        }
        fakeList["channels"] = fakeChannels;

        QJsonDocument generated(fakeList);
        responseReady(generated.toJson(QJsonDocument::Indented), true);
    }
    // channel command stubs
    else if (doc["cmd"].toString() == "fader") {
        responseReady(ok, true);
    } else if (doc["cmd"].toString() == "mute") {
        responseReady(ok, true);
    }
}

void Simulator::ping() {
    QRandomGenerator *rnd = QRandomGenerator::global();

    QByteArray fakeLive;
    fakeLive.reserve(512);

    fakeLive.append('\x01');
    fakeLive.append('\x00');
    for (int i = 0; i < 16; i++) {
        quint8 level = rnd->bounded(0, 96);
        fakeLive.append('\x43');
        fakeLive.append('\x48');
        fakeLive.append('\x00'); // fader
        fakeLive.append(level);  // level bar
        fakeLive.append('\x00'); // unused right channel
        fakeLive.append('\x00'); // balance
        fakeLive.append('\x00'); // mute
    }

    qDebug() << "Simulated live data: " << fakeLive.toHex(' ');
    emit liveDataReady(fakeLive);
}
