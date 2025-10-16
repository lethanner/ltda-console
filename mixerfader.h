#ifndef MIXERFADER_H
#define MIXERFADER_H

#include <QSlider>
#include <QPainter>
#include <QEvent>
#include <QObject>
#include <QStyle>
#include <QStyleOptionSlider>

class MixerFader : public QSlider {
  Q_OBJECT
public:
  explicit MixerFader(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
private:
    //bool dBmarkersNeedUpdate = true;
};

#endif  // MIXERFADER_H
