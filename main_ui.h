#ifndef UIDEMO08_H
#define UIDEMO08_H

#include <QWidget>
#include "bmsview.h"
#include "mb_cmu.h"
#include "models/ListView/ListView.h"
#include "myhelper.h"
#include "notifymanager.h"
#include "tftpserver.h"
#define EXIT_CODE_REBOOT 123456789

#include <AbstractAppender.h>
#include <Logger.h>

class LogAppender : public AbstractAppender {
   protected:
    void append(const QDateTime &timeStamp, Logger::LogLevel logLevel, const char *file, int line, const char *function,
                const QString &category, const QString &message) override {
        records.append({timeStamp, logLevel, file, line, function, category, message});
    }

   public:
    struct Record {
        QDateTime timeStamp;
        Logger::LogLevel logLevel;
        const char *file;
        int line;
        const char *function;
        QString category;
        QString message;
    };
    QList<Record> records;

    void clear() { records.clear(); }
};

class QToolButton;

namespace Ui {
class MainUI;
}

class MainUI : public QWidget {
    Q_OBJECT

   public:
    explicit MainUI(QWidget *parent = nullptr);
    ~MainUI();

   protected:
    void closeEvent(QCloseEvent *event);

   private:
    Ui::MainUI *ui;
    QList<int> pixCharConfig;
    QList<QToolButton *> btnsConfig;
    QTimer *timer;

    TFTPServer *tftpd;
    NotifyManager *manager;
    QSettings *settings;
    LogAppender appender;
   private slots:
    void initForm();
    void buttonClick();
    void menuClick();
};

#endif  // UIDEMO08_H
