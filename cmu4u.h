#ifndef _CMU4U_H
#define _CMU4U_H

#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "frmbalancebox.h"
#include "frmbalanceConfig.h"
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
#include "models/SOEModel/SOEModel.h"
#include <QSettings>
using namespace std;

namespace Ui {
class cmu4u;
}

class cmu4u : public QWidget {
    Q_OBJECT

   public:
    explicit cmu4u(QWidget* parent = nullptr);
    ~cmu4u();
    void flushData();
    void uiInit();
    mb_cmu* mycmu;

   private:
    Ui::cmu4u* ui;
    MessageQueue* pmq;
    QTimer* timer;
    CMU_CONF config;
    SOEModel m_model;
    QSettings* settings;
    QMenu *update_menu;

    frmBalanceBox* inputBalance = nullptr;
    frmbalanceConfig* configBalance = nullptr;
    bool eventFilter(QObject* obj, QEvent* event);
    int setValue(string name, double dval);
    bool load_config();
    bool exportExecl(QTableWidget* tableWidget, QString dirFile);
   private slots:
    void timerUpDate();
    void valueChange();
    void btn_released();
    void btn_contrl();
    void stateChanged();
    void checkChanged();
    void on_lineEditIP_editingFinished();
    void on_lineEditServIP_editingFinished();
    //
    void on_cbProtocol_currentIndexChanged(const QString& arg1);
    void btnClick();

    void on_checkBox_stateChanged(int arg1);
    void on_btnOutput_released();
    void on_btnInput_released();
    void onUpdateBtnMenu();
    void initUpdateMenu();
    void IpChange();

};

#endif
