#include "connectiondialog.h"

ConnectionDialog::InputData *ConnectionDialog::inputData = nullptr;
UART *ConnectionDialog::i_UART = nullptr;
LAN *ConnectionDialog::i_LAN = nullptr;
Simulator *ConnectionDialog::i_SIM = nullptr;

ConnectionDialog::ConnectionDialog(Device *_dev, QWidget *parent)
    : QDialog(parent), dev(_dev)
{
    setFixedSize(300, 220);
    setWindowTitle(tr("Connect..."));

    if (!i_LAN) i_LAN = new LAN();
    if (!i_UART) i_UART = new UART();
    if (!i_SIM) i_SIM = new Simulator();

    // default values
    // TODO: load from saved configuration
    if (!inputData) {
        inputData = new InputData({0, "192.168.4.1", 12344, "", 115200});
    }

    selectTcp = new QRadioButton("Through LAN", this);
    selectUart = new QRadioButton("Through UART", this);
    selectSim = new QRadioButton("Simulate", this);

    selectGroup = new QButtonGroup(this);
    selectGroup->addButton(selectTcp, 0);
    selectGroup->addButton(selectUart, 1);
    selectGroup->addButton(selectSim, 2);
    QAbstractButton* targetButton = selectGroup->button(inputData->selection);
    if (targetButton)
        targetButton->setChecked(true);

    layout = new QVBoxLayout(this);
    inputsLayout = new QHBoxLayout();
    ipLayout = new QVBoxLayout();
    portLayout = new QVBoxLayout();
    comPortLayout = new QHBoxLayout();

    bool isLanChecked = selectTcp->isChecked();
    lblIpAddr = new QLabel("IP address:", this);
    lblPort = new QLabel("Port:", this);

    inputIpAddr = new QLineEdit(this);
    inputIpAddr->setText(inputData->ip_addr);
    inputIpAddr->setEnabled(isLanChecked);
    inputPort = new QSpinBox(this);
    inputPort->setRange(1, 65535);
    inputPort->setValue(inputData->port);
    inputPort->setEnabled(isLanChecked);

    ipLayout->addWidget(lblIpAddr);
    ipLayout->addWidget(inputIpAddr);
    portLayout->addWidget(lblPort);
    portLayout->addWidget(inputPort);

    bool isUartChecked = selectUart->isChecked();
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
            getAvailableComPorts();
        refreshComPort->setEnabled(checked);
        selectComPort->setEnabled(checked);
        inputIpAddr->setEnabled(!checked);
        inputPort->setEnabled(!checked);
    });
    connect(refreshComPort, &QPushButton::clicked, [this]() {
        getAvailableComPorts();
    });
    connect(selectGroup, &QButtonGroup::idClicked, [this](int id) {
        inputData->selection = id;
    });

    inputsLayout->addLayout(ipLayout);
    inputsLayout->addLayout(portLayout);
    layout->addWidget(selectTcp);
    layout->addLayout(inputsLayout);
    layout->addWidget(selectUart);
    layout->addLayout(comPortLayout);
    layout->addWidget(selectSim);
    layout->addStretch();
    layout->addWidget(settingsConfirm);
}

void ConnectionDialog::onSave() {
    inputData->ip_addr = inputIpAddr->text();
    inputData->port = inputPort->value();
    inputData->comPort = selectComPort->currentText();

    i_UART->setPort(inputData->comPort, inputData->baudrate);
    i_LAN->setHost(inputData->ip_addr, inputData->port);

    // is this a temporarily solution?
    if (selectTcp->isChecked())
        dev->setInterface(i_LAN);
    else if (selectUart->isChecked())
        dev->setInterface(i_UART);
    else
        dev->setInterface(i_SIM);

    accept();
}

void ConnectionDialog::getAvailableComPorts() {
    QStringList ports = UART::getAvailableComPorts();

    selectComPort->clear();
    selectComPort->setEnabled(false);

    if (ports.empty())
        return;

    selectComPort->setEnabled(true);
    selectComPort->addItems(ports);
}
