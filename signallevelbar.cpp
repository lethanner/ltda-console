#include "signallevelbar.h"

SignalLevelBar::SignalLevelBar(QWidget* parent)
    : QProgressBar(parent) {
    setOrientation(Qt::Vertical);
    setMinimum(0);
    setMaximum(97);
    setTextVisible(false);
    setFixedWidth(10);
}

void SignalLevelBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    int w = width();
    int h = height();
    int val = this->value();

    double percent = (double)(val - minimum()) / (maximum() - minimum());

    p.fillRect(rect(), Qt::black);

    int filledSegments = static_cast<int>(30 * percent);
    int gapSize = 2;
    int totalGapSpace = 29 * gapSize;
    int segmentHeight = (h - totalGapSpace) / 30;

    for (int i = 0; i < 30; i++) {
        int y = h - (i * (segmentHeight + gapSize)) - segmentHeight;

        QColor color;
        if (i >= filledSegments) color = QColor(20, 20, 20);
        else if (i < 20) color = Qt::green;
        else if (i < 28) color = QColor(255, 215, 0);
        else color = Qt::red;

        QRect segmentRect(1, y, w - 2, segmentHeight);
        p.fillRect(segmentRect, color);
    }
}
