#include "connectiondialog.h"

ConnectionDialog::InputData ConnectionDialog::inputData = {
    ConnectionInterface::LAN, "192.168.4.1", 12344, "", 115200
};

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(300, 200);
    setWindowTitle(tr("Connect..."));

    layout = new QVBoxLayout(this);
    inputsLayout = new QHBoxLayout();
    ipLayout = new QVBoxLayout();
    portLayout = new QVBoxLayout();
    comPortLayout = new QHBoxLayout();

    selectTcp = new QRadioButton("Through LAN", this);
    selectTcp->setChecked(true);
    lblIpAddr = new QLabel("IP address:", this);
    lblPort = new QLabel("Port:", this);

    inputIpAddr = new QLineEdit(this);
    inputIpAddr->setText(inputData.ip_addr);
    inputPort = new QSpinBox(this);
    inputPort->setRange(1, 65535);
    inputPort->setValue(inputData.port);

    ipLayout->addWidget(lblIpAddr);
    ipLayout->addWidget(inputIpAddr);
    portLayout->addWidget(lblPort);
    portLayout->addWidget(inputPort);

    selectUart = new QRadioButton("Through UART", this);
    selectUart->setChecked(false);
    selectComPort = new QComboBox(this);
    selectComPort->setEnabled(false);
    refreshComPort = new QPushButton("Refresh", this);
    refreshComPort->setEnabled(false);

    comPortLayout->addWidget(selectComPort);
    comPortLayout->addWidget(refreshComPort);
    selectComPort->setFixedWidth(100);

    settingsConfirm = new QDialogButtonBox();
    settingsConfirm->setOrientation(Qt::Horizontal);
    settingsConfirm->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    connect(settingsConfirm, &QDialogButtonBox::accepted, this, &ConnectionDialog::onSave);
    connect(settingsConfirm, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(selectTcp, &QRadioButton::clicked, [this](bool checked){
        refreshComPort->setEnabled(!checked);
        selectComPort->setEnabled(!checked);
        inputIpAddr->setEnabled(checked);
        inputPort->setEnabled(checked);
    });
    connect(selectUart, &QRadioButton::clicked, [this](bool checked){
        if (checked)
            emit comPortListRequest();
        refreshComPort->setEnabled(checked);
        selectComPort->setEnabled(checked);
        inputIpAddr->setEnabled(!checked);
        inputPort->setEnabled(!checked);
    });
    connect(refreshComPort, &QPushButton::clicked, [this]() {
        emit comPortListRequest();
    });

    inputsLayout->addLayout(ipLayout);
    inputsLayout->addLayout(portLayout);
    layout->addWidget(selectTcp);
    layout->addLayout(inputsLayout);
    layout->addWidget(selectUart);
    layout->addLayout(comPortLayout);
    layout->addStretch();
    layout->addWidget(settingsConfirm);
}

void ConnectionDialog::onSave() {
    inputData.iface = selectTcp->isChecked() ? ConnectionInterface::LAN : ConnectionInterface::UART;
    inputData.ip_addr = inputIpAddr->text();
    inputData.port = inputPort->value();
    inputData.comPort = selectComPort->currentText();

    accept();
}

void ConnectionDialog::setAvailableComPorts(const QStringList &ports) {
    selectComPort->clear();
    selectComPort->setEnabled(false);

    if (ports.empty())
        return;

    selectComPort->addItems(ports);
    selectComPort->setEnabled(true);
}
