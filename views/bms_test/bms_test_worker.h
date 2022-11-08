#ifndef _BMS_TEST_WORKER_H
#define _BMS_TEST_WORKER_H

#include <QDebug>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMutex>
#include <QSettings>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>
#include <iostream>
#include "mb_tcp.h"

/**
 * @brief The testWorker class
 */
class bmsTestWorker : public QObject {
    Q_OBJECT
   public:
    explicit bmsTestWorker(QObject *parent = nullptr) {}
    bmsTestWorker(QString ip, const QMap<QString, double> setMap, int mode = 0) {
        m_ip = ip;
        m_setMap = setMap;
        m_mode = mode;
    }
    ~bmsTestWorker() {}

   public:
    void setServerIp(QString ServerIp) { m_ServerIp = ServerIp; }

   private:
    QMutex m_mutex;
    QString m_ip;
    QString m_ServerIp;
    int m_mode;
    QMap<QString, double> m_setMap;
   signals:
    void workFinished(int state, QString msg);
   public slots:
    void doWork() {
        qDebug() << m_mode;
        switch (m_mode) {
            default:
                doCommand(m_ip, m_mode);
                break;
        }
    }
    void doTest(QString ip, const QMap<QString, double> setMap);
    void doSetData(QString ip, const QMap<QString, double> setMap);
    void doCommand(QString ip, uint command);
    void doSetIp(QString ip, QString serverIp);
};

#endif  //_BMS_TEST_WORKER_H
