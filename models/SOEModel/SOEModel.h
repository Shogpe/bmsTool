#ifndef SOE_MODEL_H
#define SOE_MODEL_H
#include <QAbstractTableModel>
#include <QDateTime>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMap>
#include <QWidget>
#include <iostream>
#include "db_manager.h"
#include "mb_cmu.h"
using namespace std;
typedef struct {
    uint16_t val_type;  // 数值类型
    double val_factor;  // 数值比例
    QString evt_name;
    QString val_name;
    QString limit_name;
} ST_SOE_CONF;
typedef QMap<uint16_t, ST_SOE_CONF> SOE_MAP;
#define UINT16          1
#define INT16           2
#define GET_BIT(x, bit) (((x) & (1 << (bit))) >> (bit))
class SOEModel : public QAbstractTableModel {
    Q_OBJECT
    QList<CMU_SOE> m_data;

   private:
    int tag = 1;
    QMap<int, db_manager::ST_DB_SOE> soe_map;
    //    const SOE_MAP soe_map{{1, {UINT16, 0.0001, "单体电池低压告警", "最小电压", "门限电压"}},
    //                          {2, {UINT16, 0.0001, "单体电池低压故障", "最小电压", "门限电压"}},
    //                          {3, {UINT16, 0.0001, "单体电池过压告警", "最大电压", "门限电压"}},
    //                          {4, {UINT16, 0.0001, "单体电池过压故障", "最大电压", "门限电压"}},
    //                          {5, {UINT16, 0.0001, "单体压差告警", "电压压差", "门限电压"}},
    //                          {6, {UINT16, 0.0001, "单体压差故障", "电压压差", "门限电压"}},
    //                          {7, {UINT16, 0.0001, "单体电压采样线断线告警", "单体电压", "--"}},
    //                          {8, {UINT16, 0.0001, "单体电池故障复归", "单体电压", "--"}},
    //                          {9, {INT16, 0.1, "模组过温告警", "最大温度", "门限温度"}},
    //                          {10, {INT16, 0.1, "模组过温故障", "最大温度", "门限温度"}},
    //                          {11, {INT16, 0.1, "模组低温告警", "最小温度", "--"}},
    //                          {12, {INT16, 0.1, "模组低温故障 ", "最小温度", "--"}},
    //                          {13, {INT16, 0.1, "模组温差告警", "温度差值", "--"}},
    //                          {14, {INT16, 0.1, "模组温差故障", "温度差值", "--"}},
    //                          {15, {INT16, 0.1, "模组温升告警", "最大温升", "--"}},
    //                          {16, {INT16, 0.1, "模组温升故障", "最大温升", "--"}},
    //                          {17, {INT16, 0.1, "模组温度采样线断线告警", "模组温度", "--"}},
    //                          {18, {INT16, 0.1, "模组温度复归", "模组温度", "--"}},
    //                          {19, {INT16, 0.1, "极柱过温告警 ", "最大极柱温度", "--"}},
    //                          {20, {INT16, 0.1, "极柱过温故障", "最大极柱温度", "--"}},
    //                          {21, {INT16, 0.1, "极柱故障复归", "极柱温度", "--"}},
    //                          {22, {INT16, 0.1, "过流告警", "电流", "--"}},
    //                          {23, {INT16, 0.1, "过流故障", "电流", "--"}},
    //                          {24, {INT16, 0.1, "过流复归", "电流", "--"}},
    //                          {25, {INT16, 0.1, "短路故障", "电流", "--"}},
    //                          {26, {UINT16, 0.1, "电池串过压告警", "簇电压", "--"}},
    //                          {27, {UINT16, 0.1, "电池串过压故障 ", "簇电压", "--"}},
    //                          {28, {UINT16, 0.1, "电池串低压告警", "簇电压", "--"}},
    //                          {29, {UINT16, 0.1, "电池串低压故障 ", "簇电压", "--"}},
    //                          {30, {UINT16, 0.1, "电池串故障复归", "簇电压", "--"}},
    //                          {31, {UINT16, 1, "电池串绝缘故障", "绝缘电阻", "--"}},
    //                          {32, {UINT16, 1, "电池串绝缘故障复归", "绝缘电阻", "--"}},
    //                          {33, {UINT16, 1, "电池串漏电流故障", "漏电流", "--"}},
    //                          {34, {UINT16, 1, "电池串漏电流故障复归", "漏电流", "--"}},
    //                          {35, {INT16, 0.1, "开始充电", "电流", "--"}},
    //                          {36, {UINT16, 1, "停止充电", "系统状态", "--"}},
    //                          {37, {INT16, 0.1, "开始放电", "电流", "--"}},
    //                          {38, {UINT16, 1, "停止放电", "系统状态", "--"}},
    //                          {39, {UINT16, 1, "电池充满", "系统状态", "--"}},
    //                          {40, {UINT16, 1, "电池放空", "系统状态", "--"}},
    //                          {41, {UINT16, 1, "均衡启动", "均衡状态", "--"}},
    //                          {42, {UINT16, 1, "均衡关闭", "均衡状态", "--"}},
    //                          {43, {UINT16, 1, "QF分闸", "QF状态", "--"}},
    //                          {44, {UINT16, 1, "KM+分闸", "KM+状态", "--"}},
    //                          {45, {UINT16, 1, "KM-分闸", "KM-状态", "--"}},
    //                          {46, {UINT16, 1, "KM+合闸", "KM+状态", "--"}},
    //                          {47, {UINT16, 1, "KM-合闸", "KM-状态", "--"}},
    //                          {48, {UINT16, 1, "开风机", "风机状态", "--"}},
    //                          {49, {UINT16, 1, "关风机", "风机状态", "--"}},
    //                          {50, {UINT16, 1, "BMS启动  ", "--", "--"}},
    //                          {51, {UINT16, 1, "配置改变", "参数值", "--"}},
    //                          {52, {UINT16, 1, "CMU升级(校验失败)", "--", "--"}},
    //                          {53, {UINT16, 1, "CMU升级成功", "CMU版本号", "--"}},
    //                          {54, {UINT16, 1, "CMU升级失败(下载失败)", "故障代码", "--"}},
    //                          {55, {UINT16, 1, "BMU升级", "--", "--"}},
    //                          {56, {UINT16, 1, "BMU升级成功 ", "BMU版本号", "--"}},
    //                          {57, {UINT16, 1, "BMU升级失败 ", "故障代码", "--"}},
    //                          {58, {UINT16, 1, "BMU掉线", "--", "--"}},
    //                          {59, {UINT16, 1, "CCU通讯故障", "--", "--"}},
    //                          {60, {UINT16, 1, "RTU通讯故障", "--", "--"}},
    //                          {61, {UINT16, 1, "单体电压断线复归", "--", "--"}},
    //                          {62, {UINT16, 1, "温度断线复归", "--", "--"}},
    //                          {63, {UINT16, 1, "簇极柱高温保护", "最大极柱温度", "--"}},
    //                          {64, {UINT16, 1, "簇极柱高温告警", "最大极柱温度", "--"}},
    //                          {65, {UINT16, 1, "簇极柱断线告警", "温度断线状态字", "--"}},
    //                          {66, {UINT16, 1, "簇极柱复归", "模组最小/最大温度", "--"}},
    //                          {67, {UINT16, 1, "QF故障(拒动)", "断路器回采状态", "--"}},
    //                          {68, {UINT16, 1, "KM故障(拒动)", "接触器回采状态", "--"}},
    //                          {69, {UINT16, 1, "KMR合闸", "KMR回采状态", "--"}},
    //                          {70, {UINT16, 1, "KMR分闸", "KMR回采状态", "--"}},
    //                          {128, {UINT16, 1, "烟感动作", "DI输入状态", "--"}},
    //                          {129, {UINT16, 1, "水浸动作", "DI输入状态", "--"}},
    //                          {130, {UINT16, 1, "消防动作", "DI输入状态", "--"}},
    //                          {131, {UINT16, 1, "急停动作", "DI输入状态", "--"}}};

   public:
    SOEModel(QObject* parent = {}) : QAbstractTableModel{parent} {}
    int rowCount(const QModelIndex& = QModelIndex()) const override { return m_data.count(); }
    int columnCount(const QModelIndex& = QModelIndex()) const override { return 3; }
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
                    auto iter = soe_map.find(index_soe.soe_type);
                    if (iter != soe_map.end()) {
                        return QString(tr("%1, %2%3%4系统状态:[%5]"))
                            .arg(iter->evt_txt)
                            .arg(getFormatData(index_soe.soe_id, iter->evt_id, "evt_id", iter->code))
                            .arg(getFormatData(index_soe.soe_val, iter->evt_dt, "evt_dt", iter->code))
                            .arg(getFormatData(index_soe.soe_limit, iter->evt_threshold, "evt_threshold", iter->code))
                            .arg(getStatusString(index_soe.soe_stat));
                    } else {
                        return QString("未知类型(%1):ID=%2,值=%3,限值=%4,系统状态:[%5]")
                            .arg(index_soe.soe_type)
                            .arg(index_soe.soe_id)
                            .arg(index_soe.soe_val)
                            .arg(index_soe.soe_limit)
                            .arg(getStatusString(index_soe.soe_stat));
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
    bool setData(const CMU_SOE* soe, int len, db_manager::SOE_TAG tag) {
        beginResetModel();
        m_data.clear();
        db_manager::Instance()->getSOE(soe_map, tag);
        for (int i = 0; i < len; i++) {
            m_data.append(*(soe + i));
        }
        endResetModel();
        return true;
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
    QString getStatusString(uint16_t status) const {
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
        // if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
        return statusList.join("|");
    }
    QString getStatusList(uint16_t status) const {
        QStringList statusList;
        if (GET_BIT(status, 0)) statusList << "1";
        if (GET_BIT(status, 1)) statusList << "2";
        if (GET_BIT(status, 2)) statusList << "3";
        if (GET_BIT(status, 3)) statusList << "4";
        if (GET_BIT(status, 4)) statusList << "5";
        if (GET_BIT(status, 5)) statusList << "6";
        if (GET_BIT(status, 6)) statusList << "7";
        if (GET_BIT(status, 7)) statusList << "8";
        if (GET_BIT(status, 8)) statusList << "9";
        if (GET_BIT(status, 9)) statusList << "10";
        if (GET_BIT(status, 10)) statusList << "11";
        if (GET_BIT(status, 11)) statusList << "12";
        if (GET_BIT(status, 12)) statusList << "13";
        if (GET_BIT(status, 13)) statusList << "14";
        if (GET_BIT(status, 14)) statusList << "15";
        if (GET_BIT(status, 15)) statusList << "16";
        // if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
        return statusList.join("|");
    }
    typedef struct {
        QString prefix;  // 前缀
        uint16_t type;   // 数据类型
        double factor;   // 系数
        double offset;   // 偏移量
        QString suffix;  // 单位
    } ST_FORMAT;
    enum SOE_DATA_TYPE {
        SOE_U16 = 0x202,
        SOE_I16 = 0x201,
        SOE_BIT = 0x206,
        SOE_MAP = 0x207,
        SOE_BIN = 0x208,
    };

    uint16_t getDataType(QString type) const {
        QMap<QString, uint16_t> type_map = {
            {"U16", SOE_U16}, {"I16", SOE_I16}, {"BIT", SOE_BIT}, {"MAP", SOE_MAP}, {"BIN", SOE_BIN},
        };
        return type_map.value(type, 0x202);
    }
    ST_FORMAT getFormat(QString rule) const {
        QStringList formatArr = rule.split(",");
        ST_FORMAT format;
        format.prefix = QString(formatArr.at(0));
        // 默认为0，无类型则显示空白
        format.type = 0;
        format.factor = 1;
        format.offset = 0;
        format.suffix = "";
        if (formatArr.size() == 5) {
            format.prefix = QString(formatArr.at(0));
            format.type = getDataType(formatArr.at(1));
            format.factor = QString(formatArr.at(2)).toDouble();
            format.offset = QString(formatArr.at(3)).toDouble();
            format.suffix = QString(formatArr.at(4));
        } else if (formatArr.size() == 2) {
            format.prefix = QString(formatArr.at(0));
            format.type = getDataType(formatArr.at(1));
        } else if (formatArr.size() == 1) {
            format.prefix = QString(formatArr.at(0));
        }
        return format;
    }
    QString getJsonData(QString key, uint64_t raw, QString jsonStr) const {
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            if (jsonDocument.isObject()) {
                QVariantMap result = jsonDocument.toVariant().toMap();
                if (result.contains(key)) {
                    QJsonObject value = result.value(key).toJsonObject();
                    if (value.contains(QString::number(raw))) {
                        return value.value(QString::number(raw)).toString();
                    }
                }
            }
        }
        return QString("%1").arg(raw);
    }
    QString getListData(QString key, uint16_t raw, QString jsonStr) const {
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            if (jsonDocument.isObject()) {
                QVariantMap result = jsonDocument.toVariant().toMap();
                if (result.contains(key)) {
                    QStringList statusList;
                    auto value = result.value(key).toList();
                    for (int i = 0; i < value.size(); i++) {
                        if (GET_BIT(raw, i)) {
                            statusList << value.at(i).toString();
                        }
                    }
                    return statusList.join("|");
                }
            }
        }
        return getStatusList(raw);
    }
    QString getFormatData(uint64_t raw, QString rule, QString key = "", QString code = "") const {
        ST_FORMAT format = getFormat(rule);
//        qDebug() << rule << format.type << format.prefix << format.factor;
        switch (format.type) {
            case SOE_I16:
                return QString("%1:%2%3, ")
                    .arg(format.prefix)
                    .arg(int16_t(raw) * format.factor + format.offset)
                    .arg(format.suffix);
            case SOE_U16:
                return QString("%1:%2%3, ")
                    .arg(format.prefix)
                    .arg(uint16_t(raw) * format.factor + format.offset)
                    .arg(format.suffix);
            case SOE_MAP:
                return QString("%1:%2%3, ").arg(format.prefix).arg(getJsonData(key, raw, code)).arg(format.suffix);
            case SOE_BIN:
                return QString("%1:[%2]%3, ").arg(format.prefix).arg(getStatusList(raw)).arg(format.suffix);
            default:
                break;
        }

        return QString("");
    }
};

#endif
