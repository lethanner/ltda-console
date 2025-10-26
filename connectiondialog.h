#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include "device.h"
#include <QStringList>

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConnectionDialog(Device *dev, QWidget *parent = nullptr);

private slots:
    void onSave();

private:
    QVBoxLayout *layout;
    QHBoxLayout *inputsLayout;
    QVBoxLayout *ipLayout;
    QVBoxLayout *portLayout;
    QHBoxLayout *comPortLayout;

    QRadioButton* selectTcp;
    QRadioButton* selectUart;

    QComboBox *selectComPort;
    QPushButton *refreshComPort;

    QLabel *lblIpAddr;
    QLabel *lblPort;

    QLineEdit *inputIpAddr;
    QSpinBox *inputPort;

    QDialogButtonBox *settingsConfirm;

    Device* dev;

    void getAvailableComPorts();

    struct InputData {
        Device::ConnectionInterface iface;
        QString ip_addr;
        quint16 port;
        QString comPort;
        quint32 baudrate;
    };
    static InputData inputData;
};

#endif  // CONNECTIONDIALOG_H
