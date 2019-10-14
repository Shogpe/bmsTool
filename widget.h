#ifndef WIDGET_H
#define WIDGET_H

#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "mb_cmu.h"

using namespace std;
#include <QDateTime>
typedef struct {
    uint16_t val_type;  // 数值类型
    double val_factor;  // 数值比例
    QString evt_name;
    QString val_name;
} ST_SOE_CONF;
#define UINT16 1
#define INT16  2
class SOEModel : public QAbstractTableModel {
    QList<CMU_SOE> m_data;

    // private:
    const map<const uint16_t, ST_SOE_CONF> soe_conf_map = {
        {1, {UINT16, 0.0001, "单体电池低压告警", "最小电压"}},
        {2, {UINT16, 0.0001, "单体电池低压故障", "最小电压"}},
        {3, {UINT16, 0.0001, "单体电池过压告警", "最大电压"}},
        {4, {UINT16, 0.0001, "单体电池过压故障", "最大电压"}},
        {5, {UINT16, 0.0001, "单体压差告警", "电压压差"}},
        {6, {UINT16, 0.0001, "单体压差故障", "电压压差"}},
        {7, {UINT16, 0.0001, "单体电压采样线断线告警", "单体电压"}},
        {8, {UINT16, 0.0001, "单体电池故障复归", "单体电压"}},
        {9, {INT16, 0.1, "模组过温告警", "最大温度"}},
        {10, {INT16, 0.1, "模组过温故障", "最大温度"}},
        {11, {INT16, 0.1, "模组低温告警", "最小温度"}},
        {12, {INT16, 0.1, "模组低温故障 ", "最小温度"}},
        {13, {INT16, 0.1, "模组温差告警", "温度差值"}},
        {14, {INT16, 0.1, "模组温差故障", "温度差值"}},
        {15, {INT16, 0.1, "模组温升告警", "最大温升"}},
        {16, {INT16, 0.1, "模组温升故障", "最大温升"}},
        {17, {INT16, 0.1, "模组温度采样线断线告警", "模组温度"}},
        {18, {INT16, 0.1, "模组温度复归", "模组温度"}},
        {19, {INT16, 0.1, "极柱过温告警 ", "最大极柱温度"}},
        {20, {INT16, 0.1, "极柱过温故障", "最大极柱温度"}},
        {21, {INT16, 0.1, "极柱故障复归", "极柱温度"}},
        {22, {INT16, 0.1, "过流告警", "电流"}},
        {23, {INT16, 0.1, "过流故障", "电流"}},
        {24, {INT16, 0.1, "过流复归", "电流"}},
        {25, {INT16, 0.1, "短路故障", "电流"}},
        {26, {UINT16, 0.1, "电池串过压告警", "簇电压"}},
        {27, {UINT16, 0.1, "电池串过压故障 ", "簇电压"}},
        {28, {UINT16, 0.1, "电池串低压告警", "簇电压"}},
        {29, {UINT16, 0.1, "电池串低压故障 ", "簇电压"}},
        {30, {UINT16, 0.1, "电池串故障复归", "簇电压"}},
        {31, {UINT16, 1, "电池串绝缘故障", "绝缘电阻"}},
        {32, {UINT16, 1, "电池串绝缘故障复归", "绝缘电阻"}},
        {33, {UINT16, 1, "电池串漏电流故障", "漏电流"}},
        {34, {UINT16, 1, "电池串漏电流故障复归", "漏电流"}},
        {35, {INT16, 0.1, "开始充电", "电流"}},
        {36, {UINT16, 1, "停止充电", "系统状态"}},
        {37, {INT16, 0.1, "开始放电", "电流"}},
        {38, {UINT16, 1, "停止放电", "系统状态"}},
        {39, {UINT16, 1, "电池充满", "系统状态"}},
        {40, {UINT16, 1, "电池放空", "系统状态"}},
        {41, {UINT16, 1, "均衡启动", "均衡状态"}},
        {42, {UINT16, 1, "均衡关闭", "均衡状态"}},
        {43, {UINT16, 1, "QF分闸", "QF状态"}},
        {44, {UINT16, 1, "KM+分闸", "KM+状态"}},
        {45, {UINT16, 1, "KM-分闸", "KM-状态"}},
        {46, {UINT16, 1, "KM+合闸", "KM+状态"}},
        {47, {UINT16, 1, "KM-合闸", "KM-状态"}},
        {48, {UINT16, 1, "开风机", "风机状态"}},
        {49, {UINT16, 1, "关风机", "风机状态"}},
        {50, {UINT16, 1, "BMS启动  ", "--"}},
        {51, {UINT16, 1, "配置改变", "参数值"}},
        {52, {UINT16, 1, "CMU升级", "--"}},
        {53, {UINT16, 1, "CMU升级成功", "CMU 版本号"}},
        {54, {UINT16, 1, "CMU升级失败", "故障代码"}},
        {55, {UINT16, 1, "BMU升级", "--"}},
        {56, {UINT16, 1, "BMU升级成功 ", "BMU版本号"}},
        {57, {UINT16, 1, "BMU升级失败 ", "故障代码"}},
        {58, {UINT16, 1, "BMU掉线", "--"}},
        {59, {UINT16, 1, "CCU通讯故障", "保留"}},
        {60, {UINT16, 1, "RTU通讯故障", "保留"}},
    };

   public:
    SOEModel(QObject* parent = {}) : QAbstractTableModel{parent} {}
    int rowCount(const QModelIndex&) const override { return m_data.count(); }
    int columnCount(const QModelIndex&) const override { return 3; }
    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) return QVariant();

        if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        } else if (role == Qt::DisplayRole) {
            CMU_SOE index_soe = m_data.at(index.row());
            switch (index.column()) {
                case 0:
                    return QString("%1").arg(index.row());
                case 1:
                    return QDateTime::fromMSecsSinceEpoch(index_soe.soe_time).toString("yyyy-MM-dd hh:mm:ss.zzz");
                case 2: {
                    if (index_soe.soe_type < 60 && index_soe.soe_type > 0) {
                        ST_SOE_CONF soe_conf = soe_conf_map.at(index_soe.soe_type);
                        double val = 0, val_limit = 0;
                        if (soe_conf.val_type == UINT16) {
                            val = *(uint16_t*)&index_soe.soe_val * soe_conf.val_factor;
                            val_limit = *(uint16_t*)&index_soe.soe_limit * soe_conf.val_factor;
                        } else {
                            val = *(int16_t*)&index_soe.soe_val * soe_conf.val_factor;
                            val_limit = *(int16_t*)&index_soe.soe_limit * soe_conf.val_factor;
                        }
                        return QString(tr("%1: ID=%2,%3=%4,限值=%5,系统状态=%6"))
                            .arg(soe_conf.evt_name)
                            .arg(index_soe.soe_id)
                            .arg(soe_conf.val_name)
                            .arg(val)
                            .arg(val_limit)
                            .arg(index_soe.soe_stat);
                    } else {
                        return QString("Unknow soe type %1").arg(index_soe.soe_type);
                    }
                }
                default:
                    return QString("Unknow");
            }
        }
        return QVariant();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
        switch (section) {
            case 0:
                return tr("序号");
            case 1:
                return tr("时间");
            case 2:
                return tr("事件解析");
            default:
                return {};
        }
    }
    void append(const CMU_SOE& soe) {
        beginInsertRows({}, m_data.count(), m_data.count());
        m_data.append(soe);
        endInsertRows();
    }
    bool setData(const QModelIndex& index, const CMU_SOE& soe, int role = Qt::EditRole) {
        if (!index.isValid()) return false;
        if (role == Qt::EditRole && index.row() <= m_data.size()) {
            m_data.replace(index.row(), soe);
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }
};

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
    bool eventFilter(QObject* obj, QEvent* event);
   private slots:
    void timerUpDate();
    void valueChange();
    void on_btn_released();
    void on_btn_do_contrl();
};
class MyDoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT

   public:
    MyDoubleSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {}

    virtual QString textFromValue(double value) const {
        /* 4 - number of digits, 10 - base of number, '0' - pad character*/
        return QString("%1").arg(value);
    }
};
class MyTimeSpinBox : public QDoubleSpinBox {
    Q_OBJECT

   public:
    MyTimeSpinBox(QWidget* parent = 0) : QDoubleSpinBox(parent) {}

    virtual QString textFromValue(double value) const {
        /* 4 - number of digits, 10 - base of number, '0' - pad character*/
        return QDateTime::fromTime_t(value).toString("yyyy-MM-dd hh:mm:ss");
    }
};

#endif
