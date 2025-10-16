#include "wifidialog.h"

WiFiDialog::InputData WiFiDialog::inputData = {
    "COVID-19_5G_Emitter", "MakePrismaConcertsGreatAgain"
};

WiFiDialog::WiFiDialog(QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(300, 150);
    setWindowTitle(tr("Setup Wi-Fi..."));

    layout = new QVBoxLayout(this);

    lblSsid = new QLabel("SSID (leave empty to clear):", this);
    lblPassword = new QLabel("Password:", this);

    inputSsid = new QLineEdit(this);
    inputSsid->setText(inputData.ssid);
    inputPassword = new QLineEdit(this);
    inputPassword->setText(inputData.password);
    inputPassword->setEchoMode(QLineEdit::Password);

    layout->addWidget(lblSsid);
    layout->addWidget(inputSsid);
    layout->addWidget(lblPassword);
    layout->addWidget(inputPassword);

    settingsConfirm = new QDialogButtonBox();
    settingsConfirm->setOrientation(Qt::Horizontal);
    settingsConfirm->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    connect(settingsConfirm, &QDialogButtonBox::accepted, this, &WiFiDialog::onSave);
    connect(settingsConfirm, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addStretch();
    layout->addWidget(settingsConfirm);
}

void WiFiDialog::onSave() {
    inputData.ssid = inputSsid->text();
    inputData.password = inputPassword->text();

    accept();
}
