#include "bms_datalog.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include "Toast.h"
#include "iconhelper.h"
#include "myhelper.h"
#include "ui_bms_datalog.h"
BmsDataLog::BmsDataLog(QWidget *parent) : QTabWidget(parent), ui(new Ui::BmsDataLog) { ui->setupUi(this); }

BmsDataLog::~BmsDataLog() { delete ui; }
QString BmsDataLog::getStatusString(uint16_t status) {
    QStringList statusList;
    if (GET_BIT(status, 0)) statusList << "总故障";
    if (GET_BIT(status, 1)) statusList << "总告警";
    if (GET_BIT(status, 2)) statusList << "充满";
    if (GET_BIT(status, 3)) statusList << "放空";
    if (GET_BIT(status, 4)) statusList << "未初始化";
    if (GET_BIT(status, 5)) statusList << "通信故障";
    if (GET_BIT(status, 6)) statusList << "均衡";
    if (GET_BIT(status, 7)) statusList << "充电";
    if (GET_BIT(status, 8)) statusList << "放电";
    if (GET_BIT(status, 9)) statusList << "停机";
    if (GET_BIT(status, 10)) statusList << "升级";
    if (GET_BIT(status, 11)) statusList << "绝缘通信故障";
    if (GET_BIT(status, 12)) statusList << "自检故障";
    if (GET_BIT(status, 13)) statusList << "拨码故障";
    if (GET_BIT(status, 14)) statusList << "BMU故障";
    if (GET_BIT(status, 15)) statusList << "并网";
    //    if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
    return statusList.join("|");
}
QString BmsDataLog::getStatus(uint16_t status, QStringList tips) {
    QStringList statusList;
    for (int i = 0; i < tips.size(); i++) {
        if (GET_BIT(status, i)) {
            statusList << tips.at(i);
        }
    }
    return statusList.join("|");
}
int BmsDataLog::log2csv() {
    QByteArray data;
    // 烧写
    QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Read dump file"), "", ";;All Files (*)");
    if (fileName.isEmpty()) {
        return -1;
    }
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Error: Cannot read file: " << qPrintable(file.errorString());
        return -1;
    }
    data = file.readAll();
    file.close();
    int len = data.size() / sizeof(CMU_LOG);
    CMU_LOG *log = (CMU_LOG *)data.data();
    if ((data.size() % sizeof(CMU_LOG) == 0) || (data.size() / log->len == 0)) {
    }
    fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Excel(*.csv)"));
    if (fileName.isEmpty()) return -2;
    file.setFileName(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << QChar(0xfeff);
        QStringList header = {"时间戳", "电流", "电压", "漏电流", "绝缘电阻", "系统状态", "故障状态", "告警状态"};
        header << "单体电压最大值"
               << "单体电压最大值ID"
               << "单体电压最小值"
               << "单体电压最小值ID";
        header << "电池模组温度最大值"
               << "电池模组温度最大值ID"
               << "电池模组温度最小值"
               << "电池模组温度最小值ID";
        header << "PACK极柱温度最大值"
               << "PACK极柱温度最大值ID"
               << "最大单体电压差值"
               << "最大电池模组温差值";
        header << "电池模组最大温度上升速率"
               << "电池模组最大温度上升速率ID"
               << "最大模组电压"
               << "最大模组电压ID"
               << "平均单体电压"
               << "簇极柱温度断线状态";

        stream << header.join(",") << endl;
        while (len--) {
            if (log->len == 260 || log->len == sizeof(CMU_LOG)) {
                //        qDebug() << log->time << "." << log->time_ms <<
                //        log->len<<getStatusString(log->data.st.SysSta);
                uint64_t time = ((uint64_t)log->time) * 1000 + log->time_ms;
                for (int i = 0; i < 20; i++) {
                    QStringList data;
                    data << QDateTime::fromMSecsSinceEpoch(time + (i - 19) * 50).toString("yyyy-MM-dd hh:mm:ss.zzz");
                    data << QString::number(log->data.st.Idc[i] * 0.1);
                    data << QString::number(log->data.st.Udc[i] * 0.1);
                    data << QString::number(log->data.st.Ile[i] * 0.1);
                    data << QString::number(log->data.st.Rins[i] * 0.1);
                    if (i % 4 == 0) {
                        data << getStatusString(log->data.st.SysSta[i / 4]);
                        data << getStatus(log->data.st.ErrStatus[i / 4], {"0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                                          "9", "10", "11", "12", "13", "14", "15"});
                        data << getStatus(log->data.st.WarnStatus[i / 4], {"0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                                           "9", "10", "11", "12", "13", "14", "15"});
                    }
                    if (i == 19) {
                        // 3个状态量占位
                        data << "";
                        data << "";
                        data << "";
                        data << QString::number(log->data.st.u16MaxCellVolt * 0.0001);
                        data << QString::number(log->data.st.u16MaxCellVoltId);
                        data << QString::number(log->data.st.u16MinCellVolt * 0.0001);
                        data << QString::number(log->data.st.u16MinCellVoltId);

                        data << QString::number(log->data.st.i16MaxPackTemp * 0.1);
                        data << QString::number(log->data.st.u16MaxPackTempId);
                        data << QString::number(log->data.st.i16MinPackTemp * 0.1);
                        data << QString::number(log->data.st.u16MinPackTempId);
                        data << QString::number(log->data.st.i16MaxPoleTemp * 0.1);
                        data << QString::number(log->data.st.u16MaxPoleTempId);

                        data << QString::number(log->data.st.u16MaxCellVoltDiff * 0.0001);
                        data << QString::number(log->data.st.i16MaxPackTempDiff * 0.1);
                        data << QString::number(log->data.st.u16MaxTRiseRate * 0.1);
                        data << QString::number(log->data.st.u16MaxTRiseRateId);

                        data << QString::number(log->data.st.u16MaxPackVolt * 0.001);
                        data << QString::number(log->data.st.u16MaxPackVoltId);
                        data << QString::number(log->data.st.u16AvgCellVolt * 0.001);

                        data << QString::number(log->data.st.u16CPoTWireSta);
                    }
                    stream << data.join(",") << endl;
                }
            }
            log++;
        }
        file.close();
    }

    return 0;
}
void BmsDataLog::on_btnConvert_released() { log2csv(); }
