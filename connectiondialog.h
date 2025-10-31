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
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QStringList>

#include "device.h"
#include "uart.h"
#include "lan.h"

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

    QButtonGroup *selectGroup;
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
    static UART* i_UART;
    static LAN* i_LAN;

    void getAvailableComPorts();

    struct InputData {
        int selection;
        QString ip_addr;
        quint16 port;
        QString comPort;
        quint32 baudrate;
    };
    static InputData *inputData;
};

#endif  // CONNECTIONDIALOG_H
