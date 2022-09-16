#ifndef _CMU_IPVIEW_H
#define _CMU_IPVIEW_H

#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
using namespace std;

namespace Ui {
class CmuIpView;
}

class CmuIpView : public QTabWidget {
    Q_OBJECT

   public:
    explicit CmuIpView(QWidget* parent = nullptr);
    ~CmuIpView();
    void flushData();
    void uiInit();
    mb_cmu* mycmu;
   private:
    Ui::CmuIpView* ui;
    MessageQueue* pmq;
    QMenu *update_menu;
    QTimer* timer;
    CMU_CONF config;
   private slots:
    void timerUpDate();

    void on_lineBmsIP_editingFinished();
    void on_lineTftpIP_editingFinished();

    //连接控制
    void IpChange();

    void on_tbtnConnect_released();
    void on_cbProtocol_currentIndexChanged(const QString &arg1);
    //升级菜单
    void onUpdateBtnMenu();
    void initUpdateMenu();
    void slot_message_call(const QString &msg);

};

#endif
