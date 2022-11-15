#ifndef _BMS_DATALOG_H
#define _BMS_DATALOG_H

#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
using namespace std;

namespace Ui {
class BmsDataLog;
}
typedef struct {
    int16_t Idc[20];
    uint16_t Udc[20];
    int16_t Ile[20];
    uint16_t Rins[20];
    uint16_t SysSta[5];
    uint16_t ErrStatus[5];
    uint16_t WarnStatus[5];
    //    uint16_t CalData[18];
    uint16_t u16MaxCellVolt;    //单体电池电压最大值x10000
    uint16_t u16MaxCellVoltId;  //单体电池电压最大值ID

    uint16_t u16MinCellVolt;    //单体电池电压最小值
    uint16_t u16MinCellVoltId;  //单体电池电压最小值ID

    int16_t i16MaxPackTemp;     //电池模组温度最大值 	x10
    uint16_t u16MaxPackTempId;  //电池模组温度最大值ID

    int16_t i16MinPackTemp;     //电池模组温度最小值
    uint16_t u16MinPackTempId;  //电池模组温度最小值ID

    int16_t i16MaxPoleTemp;     // PACK极柱温度最大值
    uint16_t u16MaxPoleTempId;  // PACK极柱温度最大值ID

    uint16_t u16MaxCellVoltDiff;  //最大单体电压差值
    int16_t i16MaxPackTempDiff;   //最大电池模组温差值

    uint16_t u16MaxTRiseRate;    //电池模组最大温度上升速率
    uint16_t u16MaxTRiseRateId;  //电池模组最大温度上升速率ID

    uint16_t u16MaxPackVolt;    //最大模组电压
    uint16_t u16MaxPackVoltId;  //最大模组电压ID
    uint16_t u16AvgCellVolt;    //平均单体电压

    uint16_t u16CPoTWireSta;  //簇极柱温度断线状态
} _log_st;
#define DATA_LEN sizeof(_log_st)
typedef struct {
    uint32_t time;
    uint16_t len;
    uint16_t time_ms;
    union {
        uint16_t arr[DATA_LEN / 2];
        _log_st st;
    } data;
} CMU_LOG;
#define GET_BIT(x, bit) (((x) & (1 << (bit))) >> (bit))

class BmsDataLog : public QTabWidget {
    Q_OBJECT

   public:
    explicit BmsDataLog(QWidget *parent = nullptr);
    ~BmsDataLog();
    QString getStatusString(uint16_t status);
    QString getStatus(uint16_t status, QStringList tips);
    int log2csv();

   private:
    Ui::BmsDataLog *ui;

   private slots:
    void on_btnConvert_released();
};

#endif
