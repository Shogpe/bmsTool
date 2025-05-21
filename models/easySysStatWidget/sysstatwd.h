#ifndef SYSSTATWD_H
#define SYSSTATWD_H

#include <QWidget>
#include <QObject>
#include <QMap>
#include "mb_cmu.h"
#include <QLabel>

typedef struct
{
    QString ver;
    bool    onLine;
}verLabel_T;

typedef struct
{
    QString objName;
    QString statName;
    bool    flg;
}statLabel_T;

#define RESERVED_TEXT_RES   tr(" ")
#define TEXT_RED   ("color:darkRed;text-decoration:underline;font:bold;")
#define TEXT_GREEN ("color:darkGreen;")

QT_BEGIN_NAMESPACE
namespace Ui { class SysStatWd; }
QT_END_NAMESPACE

class SysStatWd : public QWidget
{
    Q_OBJECT

public:

    //所有的label对照表---这里tr内的内容一定要与bmsview中的保持一致
    QMap<QString, QString> statMapDefault = {
        {"cmuTotalFault", tr("CMU总故障")},
        {"cmuTotalALarm", tr("CMU总告警")},
        {"cmuTotalWarn",  tr("CMU总预警")},
        {"full",          tr("电池充满")},
        {"empty",         tr("电池放空")},
        {"CHG",           tr("电池充电")},
        {"DSG",           tr("电池放电")},
        {"stop",          tr("系统停机")},
        {"BMUComm",       tr("BMU通信")},
        {"INSComm",       tr("绝缘通信")},
        {"upGrade",       tr("升级标志")},
        {"CellOVLock",    tr("电芯过压锁定")},
        {"CellUVLock",    tr("电芯欠压锁定")},
        {"CellOVFault",   tr("电芯过压故障")},
        {"CellUVFault",   tr("电芯欠压故障")},
        {"CellOVAlarm",   tr("电芯过压告警")},
        {"CellUVAlarm",   tr("电芯欠压告警")},
        {"CellOTLock",    tr("电芯高温锁定")},
        {"CellUTLock",    tr("电芯低温锁定")},
        {"CellOTFault",   tr("电芯高温故障")},
        {"CellUTFault",   tr("电芯低温故障")},
        {"CellOTAlarm",   tr("电芯高温告警")},
        {"CellUTAlarm",   tr("电芯低温告警")},
        {"StringOVFault", tr("簇过压故障")},
        {"StringUVFault", tr("簇欠压故障")},
        {"StringOVAlarm", tr("簇过压告警")},
        {"StringUVAlarm", tr("簇欠压告警")},
        {"BMUCommAb"    , tr("CMU-BMU通信异常")},
        {"INSCommAb"    , tr("CMU-INS通信异常")},
        {"Grid"         , tr("并网状态")},
        {"QFStat"       , tr("断路器QF状态")},
        {"KMPStat"      , tr("接触器KM+状态")},
        {"KMNStat"      , tr("接触器KM-状态")},
        {"OCFault"      , tr("充放电过流故障")},
        {"OCAlarm"      , tr("充放电过流告警")},
    };

    //对应版本需要排除的项
    const QList<QString> exceptList[12] =
    {
        //0-CMUV1
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //1-CMUV2
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //2-CMUV3
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //3-CMUV4
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //4-CMUV4_1
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //5-CMUV4_8
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //6-CMUV3_1
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //7-CMUV4_6
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //8-CMUV4_9
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //9-CMUV4_10
        {"cmuTotalWarn","CellOVLock","CellUVLock","CellOTLock","CellUTLock","BMUCommAb","INSCommAb"},

        //10-CMUV5_0
        {"BMUComm","INSComm"},

        //11-CMUV5_1
        {"BMUComm","INSComm"},
    };

    SysStatWd(QWidget *parent = nullptr);
    ~SysStatWd();

    void clear();
    void setVerList(QList<verLabel_T> list);
    void setProtocol(BMS_PROTOCOL p) {protocol = p; initAllStatUI();}
    void initAllStatUI();
    bool isExcepted(BMS_PROTOCOL, QString objName);
    void refreashAllStat();

    void debugShowAllTrue();
    void setLabel(QString text, bool flag);

private:
    Ui::SysStatWd *ui;

    BMS_PROTOCOL protocol = CMUV3;
//    QList<verLabel_T> verList;
    QList<statLabel_T* > statList;
    QList<QLabel* > labelUIList;
};
#endif // SYSSTATWD_H
