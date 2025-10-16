#ifndef SIGNALLEVELBAR_H
#define SIGNALLEVELBAR_H

#include <QProgressBar>
#include <QPainter>

class SignalLevelBar : public QProgressBar {
  Q_OBJECT
public:
  explicit SignalLevelBar(QWidget* parent = nullptr);
protected:
  void paintEvent(QPaintEvent* event) override;
};

#endif  // SIGNALLEVELBAR_H
