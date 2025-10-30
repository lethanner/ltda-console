#include "connectiondialog.h"

ConnectionDialog::ConnectionDialog(Device *_dev, QWidget *parent)
    : QDialog(parent), dev(_dev)
{
    setFixedSize(300, 200);
    setWindowTitle(tr("Connect..."));

    // default values
    inputData.selection = selectTcp;
    inputData.ip_addr = "192.168.4.1";
    inputData.port = 12344;
    inputData.comPort = "";
    inputData.baudrate = 115200;

    layout = new QVBoxLayout(this);
    inputsLayout = new QHBoxLayout();
    ipLayout = new QVBoxLayout();
    portLayout = new QVBoxLayout();
    comPortLayout = new QHBoxLayout();

    i_LAN = new LAN();
    i_UART = new UART();

    bool isLanChecked = (inputData.selection == selectTcp);
    selectTcp = new QRadioButton("Through LAN", this);
    selectTcp->setChecked(isLanChecked);
    lblIpAddr = new QLabel("IP address:", this);
    lblPort = new QLabel("Port:", this);

    inputIpAddr = new QLineEdit(this);
    inputIpAddr->setText(inputData.ip_addr);
    inputIpAddr->setEnabled(isLanChecked);
    inputPort = new QSpinBox(this);
    inputPort->setRange(1, 65535);
    inputPort->setValue(inputData.port);
    inputPort->setEnabled(isLanChecked);

    ipLayout->addWidget(lblIpAddr);
    ipLayout->addWidget(inputIpAddr);
    portLayout->addWidget(lblPort);
    portLayout->addWidget(inputPort);

    bool isUartChecked = (inputData.selection == selectUart);
    selectUart = new QRadioButton("Through UART", this);
    selectUart->setChecked(isUartChecked);
    selectComPort = new QComboBox(this);
    selectComPort->setEnabled(isUartChecked);
    refreshComPort = new QPushButton("Refresh", this);
    refreshComPort->setEnabled(isUartChecked);

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
            i_UART->getAvailableComPorts();
        refreshComPort->setEnabled(checked);
        selectComPort->setEnabled(checked);
        inputIpAddr->setEnabled(!checked);
        inputPort->setEnabled(!checked);
    });
    connect(refreshComPort, &QPushButton::clicked, [this]() {
        i_UART->getAvailableComPorts();
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
    //inputData.iface = selectTcp->isChecked() ? Device::LAN : Device::UART;
    inputData.ip_addr = inputIpAddr->text();
    inputData.port = inputPort->value();
    inputData.comPort = selectComPort->currentText();

    i_UART->setPort(inputData.comPort, inputData.baudrate);
    i_LAN->setHost(inputData.ip_addr, inputData.port);

    if (selectTcp->isChecked())
        dev->begin(i_LAN);
    else
        dev->begin(i_UART);

    accept();
}

void ConnectionDialog::getAvailableComPorts() {
    QStringList ports = i_UART->getAvailableComPorts();

    selectComPort->clear();
    selectComPort->setEnabled(false);

    if (ports.empty())
        return;

    selectComPort->addItems(ports);
    selectComPort->setEnabled(true);
}
