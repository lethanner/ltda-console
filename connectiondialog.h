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

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    struct InputData {
        QString ip_addr;
        quint16 port;
    };
    static InputData inputData;

private slots:
    void onSave();

private:
    QVBoxLayout *layout;
    QHBoxLayout *inputsLayout;
    QVBoxLayout *ipLayout;
    QVBoxLayout *portLayout;

    QLabel *lblIpAddr;
    QLabel *lblPort;

    QLineEdit *inputIpAddr;
    QSpinBox *inputPort;

    QDialogButtonBox *settingsConfirm;
};

#endif  // CONNECTIONDIALOG_H
