#ifndef TABLE_MODEL_H
#define TABLE_MODEL_H
#include <QAbstractTableModel>
#include <QDateTime>
#include <QDebug>
#include <QMap>
#include <QWidget>
#include <iostream>

class GroupModel : public QAbstractTableModel {
    Q_OBJECT
    QList<QStringList> m_data;
    QStringList headers;

   private:
   public:
    GroupModel(QObject* parent = nullptr) : QAbstractTableModel{parent} {}
    int rowCount(const QModelIndex& = QModelIndex()) const override { return m_data.size(); }
    int columnCount(const QModelIndex& = QModelIndex()) const override { return headers.size(); }
    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) return QVariant();

        if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        } else if (role == Qt::DisplayRole) {
            if (index.row() < m_data.size()) {
                QStringList list = m_data.at(index.row());
                if (index.column() < list.size()) {
                    return list.at(index.column());
                }
            }
        }
        return QVariant();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
            return section;
        } else if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section < headers.size()) {
                return headers.at(section);
            }
        }
        return {};
    }
    void append(const QMap<QString, QString> data) {
        beginInsertRows({}, m_data.count(), m_data.count());
        QStringList list;
        for (int i = 0; i < headers.size(); i++) {
            list << data.value(headers.at(i), "-");
        }
        m_data.append(list);
        endInsertRows();
        if (m_data.size() > 500) {
            beginRemoveRows(QModelIndex(), 0, 0);
            m_data.removeFirst();
            endRemoveRows();
        }
    }
    bool setData(const QModelIndex& index, const QMap<QString, QString> data, int role = Qt::EditRole) {
        if (!index.isValid()) return false;
        if (role == Qt::EditRole && index.row() <= m_data.size()) {
            QStringList list;
            for (int i = 0; i < headers.size(); i++) {
                list << data.value(headers.at(i), "-");
            }
            m_data.replace(index.row(), list);
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }
};

#endif  // TABLE_MODEL_H
