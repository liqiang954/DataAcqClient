#include "mainwindow.h"
#include <QApplication>
#include "const.h"
#include "loghandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LogHandler::Get().installMessageHandler();
    qDebug() << "start time: " << QTime::currentTime().toString("hh:mm:ss");
    qInfo() << QString("get out! bug");
    //a.setWindowIcon(QIcon(":/main.ico"));
    QFile qss(":/style/stylesheet/stylesheet.qss");

        if( qss.open(QFile::ReadOnly))
        {
            qDebug("open success");
            QString style = QLatin1String(qss.readAll());
            a.setStyleSheet(style);
            qss.close();
        }else{
             qDebug("Open failed");
         }

    MainWindow w;
    w.setWindowTitle("数据采集上位机");
    w.show();

    int ret= a.exec();
    qDebug() << "end time: " << QTime::currentTime().toString("hh:mm:ss");
    LogHandler::Get().uninstallMessageHandler();
    return ret;
}
