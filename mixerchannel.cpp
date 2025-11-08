#include "mixerchannel.h"

MixerChannel::MixerChannel(QString name, ChannelType type, bool isStereo, quint16 number, QWidget *parent)
    : QWidget{ parent }, sequentialNo(number), _type(type) {
    setFixedWidth(70);
    setStyleSheet("background: #222");

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(6);

    label = new QLabel(name, this);
    label->setFixedHeight(20);
    label->setStyleSheet(QString("font-size:10px; color:#fff; background-color: rgb(%1, %2, %3);")
                             .arg(color[type][0]).arg(color[type][1]).arg(color[type][2]));
    label->setAutoFillBackground(true);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    balance = new QSlider(Qt::Horizontal, this);
    balance->setFixedHeight(12);
    balance->setRange(-50, 50);
    balance->setStyleSheet("QSlider { background: transparent; margin: 10% 0; }"
                           "QSlider::handle { background:#888; width:8px; margin:-2px 0; }"
                           /*"QSlider::groove { background-color: #111; }"*/);
    layout->addWidget(balance);

    faderZone = new QHBoxLayout();
    faderZone->setSpacing(1);
    fader = new MixerFader(Qt::Vertical, this);
    faderZone->addWidget(fader);

    levelL = new SignalLevelBar(this);
    levelL->setOrientation(Qt::Vertical);
    levelL->setValue(0);
    faderZone->addWidget(levelL);

    if (isStereo) {
        levelR = new SignalLevelBar(this);
        levelR->setOrientation(Qt::Vertical);
        levelR->setValue(0);
        faderZone->addWidget(levelR);
    }

    layout->addLayout(faderZone);

    dBvalue = new QLabel("off", this);
    dBvalue->setFixedHeight(16);
    dBvalue->setStyleSheet("color:#fff; font-size:9px;");
    dBvalue->setAlignment(Qt::AlignCenter);
    layout->addWidget(dBvalue);

    mute = new QPushButton("MUTE", this);
    mute->setFixedHeight(30);
    mute->setCheckable(true);
    mute->setStyleSheet("QPushButton { background:#100; color:white; }"
                        "QPushButton:checked { background:red; }");
    layout->addWidget(mute);

    connect(fader, &QSlider::sliderPressed, this, [this](){ userMovingFader = true; });
    connect(fader, &QSlider::sliderReleased, this, [this](){ userMovingFader = false; });
    connect(fader, &QSlider::valueChanged, this, [this](int value) {
        dBvalue->setText(value > -97 ? QString("%1 dB").arg(value) : "off");
        if (userMovingFader)
            emit faderMoved(sequentialNo, fader->value());
    });
    connect(mute, &QPushButton::clicked, this, [this](bool checked) {
        emit muteClicked(sequentialNo);
    });
}

void MixerChannel::synchronize(int8_t nFaderPosition, int8_t nBalance, bool nMute, int8_t nLevelL, int8_t nLevelR) {
    if (fader->value() != nFaderPosition && !userMovingFader)
        fader->setSliderPosition(nFaderPosition);
    if (balance->value() != nBalance)
        balance->setSliderPosition(nBalance);
    if (mute->isChecked() != nMute)
        mute->setChecked(nMute);

    levelL->setValue(nLevelL);
    if (levelR) levelR->setValue(nLevelR);
}
