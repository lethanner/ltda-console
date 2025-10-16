#ifndef WIFIDIALOG_H
#define WIFIDIALOG_H

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

class WiFiDialog : public QDialog {
    Q_OBJECT

public:
    explicit WiFiDialog(QWidget *parent = nullptr);
    struct InputData {
        QString ssid;
        QString password;
    };
    static InputData inputData;

private slots:
    void onSave();

private:
    QVBoxLayout *layout;

    QLabel *lblSsid;
    QLabel *lblPassword;

    QLineEdit *inputSsid;
    QLineEdit *inputPassword;

    QDialogButtonBox *settingsConfirm;
};

#endif  // WIFIDIALOG_H
