#ifndef _RTU_VIEW_H
#define _RTU_VIEW_H

#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "mb_tcp.h"
#include "rtu_soe_model.h"
using namespace std;

namespace Ui {
class RTUView;
}

class RTUView : public QTabWidget {
    Q_OBJECT

   public:
    explicit RTUView(QWidget* parent = nullptr);
    ~RTUView();
    void flushData();
    void uiInit();
    mb_tcp* m_device;
   private:
    Ui::RTUView* ui;
    QMenu *update_menu;
    QTimer* timer;
    rtuSOEModel* m_model;
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
