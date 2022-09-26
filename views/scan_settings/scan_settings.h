#ifndef _SCAN_SETTING_H
#define _SCAN_SETTING_H

#include <QDebug>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMutex>
#include <QSettings>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "mb_tcp.h"
using namespace std;

namespace Ui {
class scan_settings;
}
typedef struct {
    int index;  //
    std::string name;
    uint32_t type;
    uint32_t valtype;
    double factor;        //变比
    int64_t offset;       //变比
    std::string unit;     //单位
    std::string name_cn;  //中文名
    double val;
} ST_PARA;
Q_DECLARE_METATYPE(ST_PARA)
#define INDEX_COLUMN 0
#define NAME_COLUMN  1
#define VALUE_COLUMN 2

#define TP_VAL 0   // 数值
#define TP_HEX 1   // HEX数据,兼容BCD
#define TP_IPA 2   // IP 地址
#define TP_BIT 3   // BIT位数据
#define TP_STR 99  // 显示name_cn

enum CMD_TYPE {
    CMD_CHECK_SET = 1,
    CMD_SEND_SET,
    CMD_UP_CMU,
    CMD_UP_BMU,
    CMD_UP_INS,
    CMD_RD_VER_BMS,
    CMD_RD_VER_INS,
    CMD_LOCK_BMU,
    CMD_UNLOCK_BMU,
};
//只读单元格
class ReadOnlyDelegate : public QItemDelegate {
    Q_OBJECT
   public:
    ReadOnlyDelegate(QObject *parent = 0) : QItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        Q_UNUSED(parent);
        Q_UNUSED(index);
        Q_UNUSED(option);
        return NULL;
    }
};
//数据单元格
class ValueDelegate : public QItemDelegate {
    Q_OBJECT
   public:
    ValueDelegate(QObject *parent = 0) : QItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        Q_UNUSED(index);
        Q_UNUSED(option);
        QLineEdit *editor = new QLineEdit(parent);
        QRegExp regExp("^-?\\d{1,}(\\.[0-9]+)?");
        editor->setValidator(new QRegExpValidator(regExp, parent));
        return editor;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const {
        QString text = index.model()->data(index, Qt::DisplayRole).toString();
        QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
        lineEdit->setText(text);
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
        QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
        QString text = lineEdit->text();
        model->setData(index, text, Qt::EditRole);
    }
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }
};
class ParaModel : public QStandardItemModel {
    Q_OBJECT

   public:
    QList<ST_PARA> vals;

    ParaModel(QObject *parent = NULL) : QStandardItemModel(parent) { vals.clear(); }
    //设置表格项数据
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (!index.isValid()) return false;
        ST_PARA record = vals.at(index.row());
        if (role == Qt::EditRole) {
            record.val = value.toString().toDouble();
            vals.replace(index.row(), record);
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }
    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid()) return QVariant();
        if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        } else if (role == Qt::DisplayRole) {
            ST_PARA p = vals.at(index.row());
            switch (index.column()) {
                case 0:
                    return QString("%1").arg(index.row());
                case 1:
                    return QString().fromStdString(p.name);
                case 2: {
                    if (p.type == TP_VAL) {
                        return QString("%1").arg(p.val, 0, 'g', 6);
                    } else if (p.type == TP_HEX) {
                        return QString("%1").arg(p.val, 0, 'g', 6);
                    } else if (p.type == TP_STR) {
                        return QString::fromStdString(p.name_cn);
                    } else {
                        return QString("%1").arg(p.val, 0, 'g', 6);
                    }
                }
                default:
                    return QString("Unknow");
            }
        }
        return QVariant();
    }
    void append(const ST_PARA &para) {
        beginInsertRows({}, vals.count(), vals.count());
        insertRow(vals.count(), QModelIndex());
        vals.append(para);
        endInsertRows();
    }
    void updateData(int row, const double value) {
        //        beginInsertRows({}, vals.count(), vals.count());
        vals[row].val = value;
        //        endInsertRows();
    }
    void updateData(int row, const QString name_cn) {
        if (row < vals.count()) {
            beginResetModel();
            vals[row].name_cn = name_cn.toStdString();
            endResetModel();
        }
    }
    // 更新表格数据
    void updateData(const QList<ST_PARA> &recordList) {
        beginResetModel();
        vals.clear();
        vals.append(recordList);
        setRowCount(vals.count());
        endResetModel();
    }
    // 更新表格数据
    QList<ST_PARA> GetData() { return vals; }
    // 行数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return vals.count();
    }
};
/**
 * @brief The testWorker class
 */
class testWorker : public QObject {
    Q_OBJECT
   public:
    explicit testWorker(QObject *parent = nullptr) {}
    testWorker(QString ip, const QMap<QString, double> setMap, int mode = 0) {
        m_ip = ip;
        m_setMap = setMap;
        m_mode = mode;
    }
    ~testWorker() {}

   public:
   private:
    QMutex m_mutex;
    QString m_ip;
    int m_mode;
    QMap<QString, double> m_setMap;
   signals:
    void workFinished(int state, QString msg);
   public slots:
    void doWork() {
        qDebug() << m_mode;
        switch (m_mode) {
            case 1:
                this->doSetData(m_ip, m_setMap);
                break;
            case 2:
                this->doTest(m_ip, m_setMap);
                break;
            default:
                doCommand(m_ip, m_mode);
                break;
        }
    }
    void doTest(QString ip, const QMap<QString, double> setMap);
    void doSetData(QString ip, const QMap<QString, double> setMap);
    void doCommand(QString ip, uint command);
};
class scan_settings : public QWidget {
    Q_OBJECT

   public:
    explicit scan_settings(QWidget *parent = nullptr);
    ~scan_settings();
    void loadXml();
    void uiInit();
    mb_tcp *m_mbtcp;

   private:
    Ui::scan_settings *ui;
    ParaModel *m_para_model;
    ParaModel *m_result_model;
    QMap<QString, double> m_setMap;
    QThread m_thread;
    testWorker m_worker;
    QMutex m_mutex;
    int target_count;
    QStringList target_ips;
    QList<ST_PARA> target_result;
    void ip_analyze();
    void setBusy(bool is_busy);
   private slots:
    void on_btnWrite_released();
    void btnCtrlMenu();
    void on_connectIP_editingFinished();
};

#endif  //_SCAN_SETTING_H
