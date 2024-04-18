#ifndef BMSVIEW_H
#define BMSVIEW_H

#include <QSettings>
#include <QTableWidget>
#include <QWidget>
#include <frmSaveLog.h>
#include <iostream>
#include "frmbalanceConfig.h"
#include "frmbalancebox.h"
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
#include "models/SOEModel/SOEModel.h"
using namespace std;

namespace Ui {
class BMSView;
}

class BMSView : public QWidget {
    Q_OBJECT

   public:
    explicit BMSView(QWidget* parent = nullptr);
    ~BMSView();
    void uiChange(QHash<QString, qreal> mapData);
    mb_cmu* mycmu;

   private:
    Ui::BMSView* ui;
//    MessageQueue* pmq;
    QString m_conn;

    QTimer* timer;
    CMU_CONF config;
    uint64_t bmu_comm;
    SOEModel m_model;
    QSettings* settings;
    QMenu* update_menu;
    bool rtu_enable = false;
    QMap<int,uint8_t>fan_Speed_map;

    frmBalanceBox* inputBalance = nullptr;
    frmbalanceConfig* configBalance = nullptr;
    bool eventFilter(QObject* obj, QEvent* event);
    int setValue(QString name, double dval);
    bool load_config();
    bool saveParameters(const QString& filename);
    bool loadParameters(const QString& filename);
    bool exportExecl(QTableWidget* tableWidget, QString dirFile);
    void StartBalanceForm(void);

   private slots:
    void timerUpDate();
    void valueChange(double dval);
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
    void uiInit();
    void IpChange();
    void on_spinBoxPort_valueChanged(int arg1);
    void on_btnSaveDefault_released();
    void on_btnLoadDefault_released();
    //
    void flushData(int type, QHash<QString, qreal> mapData);
    void flushBmu();
    void flushSoe(const ST_SOE &soe);
    void pop_bmuTable_menu(const QPoint& pos);

    void on_btn_debugLog_clicked();

private:
    void changeEvent(QEvent* event);
    QString GetBitStatus(uint16_t value, QString tips = "");
    frmSaveLog savelog;
signals:
    void send_msg(TMsgData MsgCmd);
};

#endif
