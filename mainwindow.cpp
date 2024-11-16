#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include "config_modbusdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionmodbus_triggered()
{
    config_modbusDialog* dialog=new config_modbusDialog(this);
    setCentralWidget(dialog);
    dialog->show();
}


void MainWindow::on_actionplot_Items_triggered()
{



}

