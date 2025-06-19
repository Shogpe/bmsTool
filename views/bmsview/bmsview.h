#ifndef BMSVIEW_H
#define BMSVIEW_H

#include <QSettings>
#include <QTableWidget>
#include <QWidget>
#include <Rebootbmus.h>
#include <frmSaveLog.h>
#include <iostream>
#include "frmbalanceConfig.h"
#include "frmbalancebox.h"
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
#include "models/SOEModel/SOEModel.h"
#include "inputbox.h"



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
    void initUserLevelForm();
    void AOCtrlEmit(uint16_t v1, uint16_t v2, QString info);
    void AOCtrlEmit(uint16_t v1, uint16_t v2);

    Rebootbmus* rebootbmus=NULL;

    QHash<QString, qreal> pcsPowerMapDataCache;

    int bmsDebugModeCnt = 0;
    QTime timeCache;

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
    void on_cbProtocol_activated(int index);
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

    void statGroupAutoHide(QHash<QString, qreal> mapData);
    void flushData(int type, QHash<QString, qreal> mapData);
    void flushBmu();
    void flushBmuVolt();
    void flushBmuTemp();
    void flushBmuVer();
    void flushBmuEx();
    void flushSoe(const ST_SOE &soe);
    void pop_bmuTable_menu(const QPoint& pos);
    void setDoButtonText(uint16_t value, QList<QString> textList);

    void on_btn_debugLog_clicked();

    void on_cb_ErrorLog_stateChanged(int arg1);

    void on_le_ErrLogUcellLimit_editingFinished();

    void on_le_ErrLogTempLimit_editingFinished();

    void radioBtnToggledChanged(bool arg);

    void on_le_ErrLog_StdValue_editingFinished();

    void on_cB_Func2_1213_currentIndexChanged(int index);

    void on_cB_Func2_1415_currentIndexChanged(int index);

    void on_refreashIp_clicked();

    void on_pb_clearNetErrCnt_clicked();

    void on_pb_modBusHelp_clicked();

private:
    void changeEvent(QEvent* event);
    QString GetBitStatus(uint16_t value, QString tips = "");
    frmSaveLog savelog;

    uint32_t verCache;
    void findPreVer(void);


    QList<InputBox*> guestHideInputBoxList;

    QList<QLabel*> lbSysStatList;
    QList<QLabel*> lbSysStat2List;
    QList<QLabel*> lbDIStatList;
    QList<QLabel*> lbDOStatList;
    QList<QLabel*> lbErrStatList;
    QList<QLabel*> lbErrStat2List;
    QList<QLabel*> lbAlmStatList;
    QList<QLabel*> lbAlmStat2List;
    QList<QLabel*> lbWarmStatList;//preAlm
    void lbListInit();
    void fillStatLabel(QList<QLabel*> &ll, uint16_t value, QStringList sl);
    void clearStatLabel(QList<QLabel*> &ll);

public slots:
    void rebootBmus();

signals:
    void send_msg(TMsgData MsgCmd);
};


#endif
