#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <iostream>
#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QTextStream>
#include <QTextCodec>

const int g_logLimitSize = 5;

struct LogHandlerPrivate {
    LogHandlerPrivate();
    ~LogHandlerPrivate();

    // ����־�ļ� log.txt�������־�ļ����ǵ��촴���ģ���ʹ�ô������ڰ���������Ϊ yyyy-MM-dd.log�������´���һ�� log.txt
    void openAndBackupLogFile();
    void checkLogFiles(); // ��⵱ǰ��־�ļ���С
    void autoDeleteLog(); // �Զ�ɾ��30��ǰ����־

    // ��Ϣ������
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    QDir   logDir;              // ��־�ļ���
    QTimer renameLogFileTimer;  // ��������־�ļ�ʹ�õĶ�ʱ��
    QTimer flushLogFileTimer;   // ˢ���������־�ļ��Ķ�ʱ��
    QDate  logFileCreatedDate;  // ��־�ļ�������ʱ��

    static QFile *logFile;      // ��־�ļ�
    static QTextStream *logOut; // �����־�� QTextStream��ʹ�þ�̬�������Ϊ�˼��ٺ������õĿ���
    static QMutex logMutex;     // ͬ��ʹ�õ� mutex
};

class LogHandler {
public:
    void installMessageHandler();   // ��Qt��װ��Ϣ������
    void uninstallMessageHandler(); // ȡ����װ��Ϣ���������ͷ���Դ

    static LogHandler& Get() {
        static LogHandler m_logHandler;
        return m_logHandler;
    }

private:
    LogHandler();

    LogHandlerPrivate *d;
};

#endif // LOGHANDLER_H