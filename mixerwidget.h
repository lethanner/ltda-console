#ifndef MIXERWIDGET_H
#define MIXERWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include "device.h"
#include "mixerchannel.h"

class MixerWidget : public QWidget {
  Q_OBJECT
public:
  explicit MixerWidget(Device* device, QWidget *parent = nullptr);
    void clear();
    void load();

signals:

private slots:
    void processLiveData(QByteArray data);

private:
    QHBoxLayout* layout;
    Device* dev;
};

#endif  // MIXERWIDGET_H
