#include "config_modbusdialog.h"
#include "ui_config_modbusdialog.h"
#include <QSerialPortInfo>
#include "const.h"

config_modbusDialog::config_modbusDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::config_modbusDialog)
{
    ui->setupUi(this);
    config_modbus_init();
}

config_modbusDialog::~config_modbusDialog()
{
    delete ui;
}

void config_modbusDialog::config_modbus_init()
{
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        ui->cbox_COM_port->addItem(port.portName());
    }
    int index=ui->cbox_COM_port->findText(Config::PortName,Qt::MatchStartsWith);
    ui->cbox_COM_port->setCurrentIndex(index);


    QList<int> list_baud={9600,19200};
    ui->cbox_Baud->clear();
    for(int i=0;i<list_baud.size();++i)
    {
        ui->cbox_Baud->addItem(QString::number(list_baud[i])+" Baud");
    }
    auto valnumber=QString::number(Config::BaudRate);
    index=ui->cbox_Baud->findText(QString::number(Config::BaudRate),Qt::MatchStartsWith);
    ui->cbox_Baud->setCurrentIndex(index);

    index=ui->cbox_DataBits->findText(QString::number(Config::DataBit),Qt::MatchStartsWith);
    ui->cbox_DataBits->setCurrentIndex(index);

    index=ui->cbox_Parity->findText(Config::Parity,Qt::MatchStartsWith);
    ui->cbox_Parity->setCurrentIndex(index);

    index=ui->cbox_StopBits->findText(QString::number(Config::StopBit),Qt::MatchStartsWith);
    ui->cbox_StopBits->setCurrentIndex(index);
}

bool config_modbusDialog::save_config()
{
    //waiting to be implemented,Not enough time
    QString str_com=ui->cbox_COM_port->currentText();
    Config::PortName=str_com;

}

void config_modbusDialog::on_btn_OK_clicked()
{
    //check com port,qt弹出警告对话框,显示严重的错误信息
    if(ui->cbox_COM_port->currentText()=="None")
    {
        QMessageBox::critical(this, "Config Error", "please COM port ");
        return;
    }

    //save config and deleter
    save_config();
    this->close();
    this->deleteLater();
}


void config_modbusDialog::on_btn_Cancel_clicked()
{
    this->close();
}

