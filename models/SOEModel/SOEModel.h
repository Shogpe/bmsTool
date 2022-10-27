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
                return QString("%1:%2%3, ")
                    .arg(format.prefix)
                    .arg(getJsonData(key, raw + format.offset, code))
                    .arg(format.suffix);
            case SOE_BIN:
                return QString("%1:[%2]%3, ").arg(format.prefix).arg(getStatusList(raw)).arg(format.suffix);
            default:
                break;
        }

        return QString("");
    }
};

#endif
