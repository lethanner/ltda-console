#include "mixerfader.h"

MixerFader::MixerFader(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent) {
    //setAttribute(Qt::WA_TransparentForMouseEvents);
    //installEventFilter(this);

    setStyleSheet(R"(
                    QSlider::groove:vertical {
                        border: 1px solid #111;
                        width: 2px;
                        margin: 24px 36px;
                    }
                    QSlider::handle:vertical {
                        image: url(:/img/knob.png);
                        margin: -24px -36px;
                        height: -1px;
                    })");
}

void MixerFader::paintEvent(QPaintEvent *event) {
    QSlider::paintEvent(event);

    if (orientation() != Qt::Vertical)
        return;

    QPainter painter(this);

    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);

    int step = singleStep();
    int minVal = minimum();
    int maxVal = maximum();
    int grooveHeight = grooveRect.height();

    int x = grooveRect.center().x();

    painter.setPen(QColor(150, 150, 150));

    for (int i = minVal; i < maxVal; i += step) {
        // Calculate position using division and rounding for accurate placement
        double ratio = double(i - minVal) / double(maxVal - minVal);
        int y = grooveRect.bottom() - qRound(ratio * grooveHeight);

        if (i == 5) {
            // Calculate height for the rectangles using the same accurate method
            double hRatio = double(5 * step) / double(maxVal - minVal);
            int h = qRound(hRatio * grooveHeight);

            painter.drawRect(x - 20, y, 6, h - 2);
            painter.drawRect(x - 20, y + h + 4, 6, h - 4);
            painter.drawRect(x + 14, y, 6, h - 2);
            painter.drawRect(x + 14, y + h + 4, 6, h - 4);
        }
        if ((i < -5 || i > 5)) {
            if (i % 10 == 0) {
                painter.drawLine(x - 20, y, x - 14, y);
                painter.drawLine(x + 20, y, x + 14, y);
            }
            else if (i % 2 == 0) {
                painter.drawLine(x - 20, y, x - 17, y);
                painter.drawLine(x + 20, y, x + 17, y);
            }
        }
    }
}
