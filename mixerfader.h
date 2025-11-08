#ifndef MIXERFADER_H
#define MIXERFADER_H

#include <QSlider>
#include <QObject>

class MixerFader : public QSlider {
  Q_OBJECT
public:
  explicit MixerFader(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif  // MIXERFADER_H
