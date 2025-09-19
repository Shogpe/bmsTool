#include "logmanager.h"
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>

LogManager::LogManager() {
#ifdef QT_NO_DEBUG
    m_format = "[%{time}{yyyy-MM-dd HH:mm:ss.zzz}][%{type:-7}] %{message}\n";
#else
    m_format = "%{time}{dd-MM-yyyy, HH:mm:ss.zzz} [%{type:-7}] [%{file:-25} %{line}] %{message}\n";
#endif
}

void LogManager::initConsoleAppender() {
    m_consoleAppender = new ConsoleAppender;
    m_consoleAppender->setFormat(m_format);
    cuteLogger->registerAppender(m_consoleAppender);
}
void LogManager::initSignalAppender() {
    cuteLogger->removeAppender(m_signalAppender);
    if (!m_signalAppender) {
        m_signalAppender = new SignalAppender;
        m_signalAppender->setLoggerLevel(Logger::Debug);
        m_signalAppender->setFormat("[%{time}{yyyy-MM-dd HH:mm:ss.zzz}][%{type:-7}] %{message}");
    }
    cuteLogger->registerAppender(m_signalAppender);
}
void LogManager::removeSignalAppender() {
    if (m_signalAppender) {
        cuteLogger->removeAppender(m_signalAppender);
    }
}
void LogManager::initRollingFileAppender() {
    QString cachePath = "logs";  // QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    if (!QDir(cachePath).exists()) {
        QDir(cachePath).mkpath(".");
    }
    m_logPath = joinPath(cachePath, QString("%1.log").arg(qApp->applicationName()));
    m_rollingFileAppender = new RollingFileAppender(m_logPath);
    qint64 maxSzie = QSettings("config.ini", QSettings::IniFormat).value("log/max_size", 200 * 1024 * 1024).toUInt();
    m_rollingFileAppender->setLogFilesMaxSize(maxSzie);
    m_rollingFileAppender->setFormat(m_format);
    m_rollingFileAppender->setLogFilesLimit(5);
    m_rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);
    cuteLogger->registerAppender(m_rollingFileAppender);
}

void LogManager::debug_log_console_on(int flag) {
    if (flag & LOG_CONSOLE) LogManager::instance()->initConsoleAppender();
    //    if (flag & LOG_WIDGET) LogManager::instance()->initSignalAppender();
    if (flag & LOG_FILE) LogManager::instance()->initRollingFileAppender();
}

QString LogManager::joinPath(const QString &path, const QString &fileName) {
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

QString LogManager::getlogFilePath() { return m_logPath; }

LogManager::~LogManager() {}
