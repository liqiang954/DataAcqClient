#include "LogHandler.h"

/************************************************************************************************************
 *                                               LogHandlerPrivate                                          *
 ***********************************************************************************************************/
// ��ʼ�� static ����
QMutex LogHandlerPrivate::logMutex;
QFile* LogHandlerPrivate::logFile = nullptr;
QTextStream* LogHandlerPrivate::logOut = nullptr;

LogHandlerPrivate::LogHandlerPrivate() {
    logDir.setPath("log"); // TODO: ��־�ļ��е�·����Ϊ exe ����Ŀ¼�µ� log �ļ��У��ɴ������ļ���ȡ
    QString logPath = logDir.absoluteFilePath("today.log"); // ��ȡ��־��·��

    // ========��ȡ��־�ļ�������ʱ��========
    // QFileInfo::created(): On most Unix systems, this function returns the time of the last status change.
    // ���Բ�������ʱʹ�����������鴴��ʱ�䣬��Ϊ��������ʱ�仯�������ڳ�������ʱ��������־�ļ�������޸�ʱ�䣬
    logFileCreatedDate = QFileInfo(logPath).lastModified().date(); // ����־�ļ������ڣ�����nullptr

    // ����־�ļ���������ǵ��촴���ģ�����������־�ļ�
    openAndBackupLogFile();

    // ʮ���Ӽ��һ����־�ļ�����ʱ��
    renameLogFileTimer.setInterval(1000 *  2); // TODO: �ɴ������ļ���ȡ
    renameLogFileTimer.start();
    QObject::connect(&renameLogFileTimer, &QTimer::timeout, [this] {
        QMutexLocker locker(&LogHandlerPrivate::logMutex);
        openAndBackupLogFile(); // ����־�ļ�
        checkLogFiles(); // ��⵱ǰ��־�ļ���С
        autoDeleteLog(); // �Զ�ɾ��30��ǰ����־
    });

    // ��ʱˢ����־������ļ��������������־�ļ��￴�����µ���־
    flushLogFileTimer.setInterval(1000); // TODO: �ɴ������ļ���ȡ
    flushLogFileTimer.start();
    QObject::connect(&flushLogFileTimer, &QTimer::timeout, [] {
        // qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"); // ���Բ�ͣ��д�����ݵ���־�ļ�
        QMutexLocker locker(&LogHandlerPrivate::logMutex);
        if (nullptr != logOut) {
            logOut->flush();
        }
    });
}

LogHandlerPrivate::~LogHandlerPrivate() {
    if (nullptr != logFile) {
        logFile->flush();
        logFile->close();
        delete logOut;
        delete logFile;

        // ��Ϊ������ static ����
        logOut  = nullptr;
        logFile = nullptr;
    }
}

// ����־�ļ� log.txt��������ǵ��촴���ģ���ʹ�ô������ڰ���������Ϊ yyyy-MM-dd.log�������´���һ�� log.txt
void LogHandlerPrivate::openAndBackupLogFile() {
    // �����߼�:
    // 1. ��������ʱ logFile Ϊ nullptr����ʼ�� logFile���п�����ͬһ����Ѿ����ڵ� logFile������ʹ�� Append ģʽ
    // 2. logFileCreatedDate is nullptr, ˵����־�ļ��ڳ���ʼʱ�����ڣ����Լ�¼�´���ʱ��
    // 3. ��������ʱ������ logFile �Ĵ������ں͵�ǰ���ڲ���ȣ���ʹ�����Ĵ���������������Ȼ��������һ���µ� log.txt �ļ�
    // 4. �����־�ļ����� LOGLIMIT_NUM ����ɾ�������
    // ��ע��log.txt ʼ��Ϊ�������־�ļ������ڶ��죬��ִ�е�3������ʹ�� log.txt �Ĵ���������������

    // �����־����Ŀ¼�����ڣ��򴴽�
    if (!logDir.exists()) {
        logDir.mkpath("."); // ���Եݹ�Ĵ����ļ���
    }
    QString logPath = logDir.absoluteFilePath("today.log"); // log.txt��·��

    // [[1]] ����ÿ������ʱ logFile Ϊ nullptr
    if (logFile == nullptr) {
        logFile = new QFile(logPath);
        logOut  = (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) ?  new QTextStream(logFile) : nullptr;
        if (logOut != nullptr)
            logOut->setCodec("UTF-8");

        // [[2]] ����ļ��ǵ�һ�δ������򴴽���������Ч�ģ���������Ϊ��ǰ����
        if (logFileCreatedDate.isNull()) {
            logFileCreatedDate = QDate::currentDate();
        }
    }

    // [[3]] ��������ʱ����������ڲ��ǵ�ǰ���ڣ���ʹ�ô���������������������һ���µ� log.txt
    if (logFileCreatedDate != QDate::currentDate()) {
        logFile->flush();
        logFile->close();
        delete logOut;
        delete logFile;

        QString newLogPath = logDir.absoluteFilePath(logFileCreatedDate.toString("yyyy-MM-dd.log"));;
        QFile::copy(logPath, newLogPath); // Bug: ����˵ rename ������ʣ����� rename ʱ���һ���ļ�������ʾ����������Ҫ killall Finder ��ų���
        QFile::remove(logPath); // ɾ�����´������ı䴴��ʱ��

        // ���´��� log.txt
        logFile = new QFile(logPath);
        logOut  = (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) ?  new QTextStream(logFile) : nullptr;
        logFileCreatedDate = QDate::currentDate();
        if (logOut != nullptr)
            logOut->setCodec("UTF-8");
    }
}

// ��⵱ǰ��־�ļ���С
void LogHandlerPrivate::checkLogFiles() {
    // ��� protocal.log �ļ���С����5M�����´���һ����־�ļ���ԭ�ļ��浵Ϊyyyy-MM-dd_hhmmss.log
    if (logFile->size() > 1024*g_logLimitSize) {
        logFile->flush();
        logFile->close();
        delete logOut;
        delete logFile;

        QString logPath = logDir.absoluteFilePath("today.log"); // ��־��·��
        QString newLogPath = logDir.absoluteFilePath(logFileCreatedDate.toString("yyyy-MM-dd.log"));
        QFile::copy(logPath, newLogPath); // Bug: ����˵ rename ������ʣ����� rename ʱ���һ���ļ�������ʾ����������Ҫ killall Finder ��ų���
        QFile::remove(logPath); // ɾ�����´������ı䴴��ʱ��

        logFile = new QFile(logPath);
        logOut  = (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) ?  new QTextStream(logFile) : NULL;
        logFileCreatedDate = QDate::currentDate();
        if (logOut != nullptr)
            logOut->setCodec("UTF-8");
    }
}

// �Զ�ɾ��30��ǰ����־
void LogHandlerPrivate::autoDeleteLog()
{
    QDateTime now = QDateTime::currentDateTime();

    // ǰ30��
    QDateTime dateTime1 = now.addDays(-30);
    QDateTime dateTime2;

    QString logPath = logDir.absoluteFilePath("today.log"); // ��־��·��
    QDir dir(logPath);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo f, fileList ) {
        // "."��".."����
        if (f.baseName() == "")
            continue;

        dateTime2 = QDateTime::fromString(f.baseName(), "yyyy-MM-dd");
        if (dateTime2 < dateTime1) { // ֻҪ��־ʱ��С��ǰ30���ʱ���ɾ��
            dir.remove(f.absoluteFilePath());
        }
    }
}

// ��Ϣ������
void LogHandlerPrivate::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker locker(&LogHandlerPrivate::logMutex);
    QString level;

    switch (type) {
    case QtDebugMsg:
        level = "DEBUG";
        break;
    case QtInfoMsg:
        level = "INFO ";
        break;
    case QtWarningMsg:
        level = "WARN ";
        break;
    case QtCriticalMsg:
        level = "ERROR";
        break;
    case QtFatalMsg:
        level = "FATAL";
        break;
    default:
        break;
    }

    // �������׼���: Windows �� std::cout ʹ�� GB2312���� msg ʹ�� UTF-8�����ǳ���� Local Ҳ����ʹ�� UTF-8
#if defined(Q_OS_WIN)
    QByteArray localMsg = QTextCodec::codecForName("GB2312")->fromUnicode(msg); //msg.toLocal8Bit();
#else
    QByteArray localMsg = msg.toLocal8Bit();
#endif

    std::cout << std::string(localMsg) << std::endl;

    if (nullptr == LogHandlerPrivate::logOut) {
        return;
    }

    // �������־�ļ�, ��ʽ: ʱ�� - [Level] (�ļ���:����, ����): ��Ϣ
    QString fileName = context.file;
    int index = fileName.lastIndexOf(QDir::separator());
    fileName = fileName.mid(index + 1);

    (*LogHandlerPrivate::logOut) << QString("%1 - [%2] (%3:%4, %5): %6\n")
                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(level)
                                    .arg(fileName).arg(context.line).arg(context.function).arg(msg);
}

/************************************************************************************************************
 *                                               LogHandler                                                 *
 ***********************************************************************************************************/
LogHandler::LogHandler() : d(nullptr) {
}

// ��Qt��װ��Ϣ������
void LogHandler::installMessageHandler() {
    QMutexLocker locker(&LogHandlerPrivate::logMutex); // ����C++11��lock_guard������ʱ�Զ�����

    if (nullptr == d) {
        d = new LogHandlerPrivate();
        qInstallMessageHandler(LogHandlerPrivate::messageHandler); // �� Qt ��װ�Զ�����Ϣ������
    }
}

// ȡ����װ��Ϣ���������ͷ���Դ
void LogHandler::uninstallMessageHandler() {
    QMutexLocker locker(&LogHandlerPrivate::logMutex);

    qInstallMessageHandler(nullptr);
    delete d;
    d = nullptr;
}