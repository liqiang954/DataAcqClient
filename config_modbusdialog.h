#ifndef CONFIG_MODBUSDIALOG_H
#define CONFIG_MODBUSDIALOG_H

#include <QDialog>

namespace Ui {
class config_modbusDialog;
}

class config_modbusDialog : public QDialog
{
    Q_OBJECT

public:
    explicit config_modbusDialog(QWidget *parent = nullptr);
    ~config_modbusDialog();

    void config_modbus_init();
    bool save_config();

private slots:
    void on_btn_OK_clicked();

    void on_btn_Cancel_clicked();

private:
    Ui::config_modbusDialog *ui;
};

#endif // CONFIG_MODBUSDIALOG_H
