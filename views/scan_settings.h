#ifndef _SCAN_SETTING_H
#define _SCAN_SETTING_H

#include <QSettings>
#include <QTableWidget>
#include <QWidget>
#include <QMutex>
#include <iostream>
#include "mb_tcp.h"
using namespace std;

namespace Ui {
class scan_settings;
}
/**
 * @brief The testWorker class
 */
class testWorker : public QObject {
    Q_OBJECT
   public:
    explicit testWorker(QObject* parent = nullptr);
    ~testWorker();

   public:
   private:
    QMutex m_mutex;
   signals:
    void done(int state, QString msg);
   public slots:
    void doTest(QString ip) {}
};
class scan_settings : public QWidget {
    Q_OBJECT

   public:
    explicit scan_settings(QWidget* parent = nullptr);
    ~scan_settings();
    void flushData();
    void uiInit();
    mb_tcp* m_mbtcp;

   private:
    Ui::scan_settings* ui;

   private slots:

};

#endif  //_SCAN_SETTING_H
