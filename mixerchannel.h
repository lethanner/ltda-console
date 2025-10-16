#ifndef MIXERCHANNEL_H
#define MIXERCHANNEL_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QPushButton>
#include "signallevelbar.h"
#include "mixerfader.h"

class MixerChannel : public QWidget {
  Q_OBJECT
public:
  explicit MixerChannel(QString name, QColor color, bool isStereo, quint16 number, QWidget *parent = nullptr);
    const quint16 getNumber() { return sequentialNo; }
  void synchronize(int8_t nFaderPosition, int8_t nBalance, bool nMute, int8_t nLevelL, int8_t nLevelR);

private:
  QVBoxLayout* layout;
    QLabel* label;
  QSlider* balance;

    QHBoxLayout* faderZone;
  MixerFader* fader;
  SignalLevelBar* levelL;
  SignalLevelBar* levelR = nullptr;
  bool userMovingFader = false;

    QLabel* dBvalue;
    QPushButton* mute;

    const quint16 sequentialNo;

signals:
    void muteClicked(const quint16 number);
    void faderMoved(const quint16 number, int8_t value);
};

#endif  // MIXERCHANNEL_H
