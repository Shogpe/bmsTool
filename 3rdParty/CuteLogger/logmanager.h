#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <AbstractStringAppender.h>
#include <Logger.h>
#include <QtCore>
class ConsoleAppender;
class RollingFileAppender;
class SignalAppender : public QObject, public AbstractStringAppender {
    Q_OBJECT
   public:
    explicit SignalAppender(QObject* parent = nullptr) : QObject(parent), AbstractStringAppender() { setFormat("[%{type:-7}] <%{function}> %{message}\n"); }
    void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line, const char* function, const QString& category,
                const QString& message) {
        QMutexLocker locker(&m_signalLock);
        if (logLevel < m_lvl) return;
        QString messageStr = formattedString(timeStamp, logLevel, file, line, function, category, message);
        emit logMessage(logLevel, messageStr);
    }
    void setLoggerLevel(Logger::LogLevel logLevel) {
        QMutexLocker locker(&m_signalLock);
        m_lvl = logLevel;
    }
   signals:
    void logMessage(int, QString);

   private:
    Logger::LogLevel m_lvl;
    mutable QMutex m_signalLock;
};
class LogManager {
    enum LogFlag {
        LOG_CONSOLE = 0x01,
        LOG_WIDGET = 0x01 << 1,
        LOG_FILE = 0x01 << 2,
    };

   public:
    void initConsoleAppender();
    void initSignalAppender();
    void removeSignalAppender();
    void initRollingFileAppender();
    SignalAppender* m_signalAppender = nullptr;

    inline static LogManager* instance() {
        static LogManager instance;
        return &instance;
    }

    void debug_log_console_on(int flag = LOG_WIDGET);
    QString joinPath(const QString& path, const QString& fileName);
    QString getlogFilePath();
    //    void setMaxFileSize(int maximum) { m_rollingFileAppender->setLogFilesMaxSize(maximum); }
   signals:

   public slots:

   private:
    QString m_format;
    QString m_logPath;
    ConsoleAppender* m_consoleAppender = nullptr;
    RollingFileAppender* m_rollingFileAppender = nullptr;
    explicit LogManager();
    ~LogManager();
    LogManager(const LogManager&);
    LogManager& operator=(const LogManager&);
};

#endif  // LOGMANAGER_H
