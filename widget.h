#ifndef WIDGET_H
#define WIDGET_H

#include <QSettings>
#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "frmbalanceConfig.h"
#include "frmbalancebox.h"
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
#include "models/SOEModel/SOEModel.h"
using namespace std;

namespace Ui {
class Widget;
}

class Widget : public QWidget {
    Q_OBJECT

   public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget();
    void flushData();
    void uiInit();
    mb_cmu* mycmu;

   private:
    Ui::Widget* ui;
    MessageQueue* pmq;
    QTimer* timer;
    CMU_CONF config;
    SOEModel m_model;
    QSettings* settings;
    QMenu* update_menu;

    frmBalanceBox* inputBalance = nullptr;
    frmbalanceConfig* configBalance = nullptr;
    bool eventFilter(QObject* obj, QEvent* event);
    int setValue(string name, double dval);
    bool load_config();
    bool saveParameters(const QString& filename);
    bool loadParameters(const QString& filename);
    bool exportExecl(QTableWidget* tableWidget, QString dirFile);
   private slots:
    void timerUpDate();
    void valueChange();
    void sendCommand();
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
    void on_spinBoxPort_valueChanged(int arg1);
    void on_btnSaveDefault_released();
    void on_btnLoadDefault_released();
};

#endif
