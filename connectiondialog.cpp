#include "connectiondialog.h"

ConnectionDialog::InputData ConnectionDialog::inputData = {
    "192.168.4.1", 12344
};

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(300, 100);
    setWindowTitle(tr("Connect..."));

    layout = new QVBoxLayout(this);
    inputsLayout = new QHBoxLayout();
    ipLayout = new QVBoxLayout();
    portLayout = new QVBoxLayout();

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

    settingsConfirm = new QDialogButtonBox();
    settingsConfirm->setOrientation(Qt::Horizontal);
    settingsConfirm->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    connect(settingsConfirm, &QDialogButtonBox::accepted, this, &ConnectionDialog::onSave);
    connect(settingsConfirm, &QDialogButtonBox::rejected, this, &QDialog::reject);

    inputsLayout->addLayout(ipLayout);
    inputsLayout->addLayout(portLayout);
    layout->addLayout(inputsLayout);
    layout->addStretch();
    layout->addWidget(settingsConfirm);
}

void ConnectionDialog::onSave() {
    inputData.ip_addr = inputIpAddr->text();
    inputData.port = inputPort->value();

    accept();
}
