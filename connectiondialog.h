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
#include <QStringList>

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    enum class ConnectionInterface {
        LAN, UART
    };

    struct InputData {
        ConnectionInterface iface;
        QString ip_addr;
        quint16 port;
        QString comPort;
        quint32 baudrate;
    };
    static InputData inputData;

    void setAvailableComPorts(const QStringList &ports);

private slots:
    void onSave();

signals:
    void comPortListRequest();

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
};

#endif  // CONNECTIONDIALOG_H
