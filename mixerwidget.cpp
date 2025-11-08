#include "mixerwidget.h"
#include <QSignalBlocker>

MixerWidget::MixerWidget(Device* device, QWidget *parent)
    : QWidget{ parent }, dev(device) {
    layout = new QHBoxLayout(this);

    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    connect(dev, &Device::liveDataReady, this, &MixerWidget::processLiveData);
}

void MixerWidget::clear() {
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void MixerWidget::processLiveData(QByteArray data) {
    if (data.at(0) != 0x01) {
        qWarning() << "Invalid streaming protocol version!";
        return;
    }

    int pos = 2;
    for (int i = 0; i < layout->count() - 1; ++i) {
        // check channel signature
        if (data[pos++] != 'C' || data[pos++] != 'H') {
            qWarning() << "Malformed packet received!";
            break;
        }

        QLayoutItem *item = layout->itemAt(i);
        if (item && item->widget()) {
            MixerChannel *channel = qobject_cast<MixerChannel*>(item->widget());
            int8_t faderPos = data[pos++], levelL = data[pos++],
                levelR = data[pos++], balance = data[pos++];
            bool mute = static_cast<bool>(data[pos++] & 0x01);
            channel->synchronize(faderPos, balance, mute, levelL, levelR);
        }
    }
}

void MixerWidget::load() {
    QJsonObject data = dev->requestMixerData();
    if (data.isEmpty()) {
        qWarning() << "Failed to load device channels";
        return;
    }

    if (data.contains("channels") && data["channels"].isArray()) {
        QJsonArray channels = data["channels"].toArray();

        quint16 index = 0;
        for (const auto& channel : channels) {
            if (channel.isObject()) {
                QJsonObject obj = channel.toObject();
                qDebug() << "Appending channel: " << obj["n"].toString();

                QColor color;
                switch (obj["t"].toInt()) {
                case 0:
                    color.setRgb(50, 150, 50);
                    break;
                case 1:
                    color.setRgb(200, 0, 0);
                    break;
                case 2:
                    color.setRgb(170, 170, 0);
                    break;
                case 3:
                    color.setRgb(0, 0, 0);
                    break;
                }

                MixerChannel* newChannel = new MixerChannel(obj["n"].toString(),
                                                            color, obj["st"].toBool(),
                                                            index, this);
                layout->addWidget(newChannel);
                connect(newChannel, &MixerChannel::faderMoved, this, [this, newChannel](const quint16 number, int8_t value){
                    QSignalBlocker blocker(newChannel);
                    dev->setFaderPosition(number, value);
                });
                connect(newChannel, &MixerChannel::muteClicked, this, [this](const quint16 number){
                    dev->toggleMute(number);
                });
            }
            index++;
        }
        layout->addStretch();
    }
}
