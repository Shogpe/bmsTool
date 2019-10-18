#pragma once

#include <QEvent>
#include <QHoverEvent>
#include <QListView>
#include <QStandardItemModel>
#include "StyledDelegate.h"
class StringListModel : public QAbstractListModel {
    Q_OBJECT

   public:
    StringListModel(QObject *parent = {}) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const { return stringList.count(); }

    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid()) return QVariant();

        if (index.row() >= stringList.size()) return QVariant();

        if (role == Qt::DisplayRole)

            return stringList.at(index.row());

        else

            return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation,

                        int role = Qt::DisplayRole) const {
        if (role != Qt::DisplayRole) return QVariant();

        if (orientation == Qt::Horizontal)

            return QString("Column %1").arg(section);

        else

            return QString("Row %1").arg(section);
    }
    bool setData(const QModelIndex &index,

                 const QVariant &value, int role)

    {
        if (index.isValid() && role == Qt::EditRole) {
            stringList.replace(index.row(), value.toString());

            emit dataChanged(index, index);

            return true;
        }

        return false;
    }
    bool insertRows(int position, int rows, const QModelIndex &parent=QModelIndex())

    {
        beginInsertRows(QModelIndex(), position, position + rows - 1);

        for (int row = 0; row < rows; ++row) {
            stringList.insert(position, "aaa");
        }

        endInsertRows();

        return true;
    }
    bool removeRows(int position, int rows, const QModelIndex &parent)

    {
        beginRemoveRows(QModelIndex(), position, position + rows - 1);

        for (int row = 0; row < rows; ++row) {
            stringList.removeAt(position);
        }

        endRemoveRows();

        return true;
    }

   private:
    QStringList stringList;
};

class ListView : public QListView {
    Q_OBJECT

   public:
    ListView(QWidget *parent = Q_NULLPTR);
    ~ListView();

   signals:
    // 点击Item信号
    void signalClicked(const QModelIndex &iIndex);
    // 点击Item信号 具体到点到的Role
    void signalClicked(const QModelIndex &iIndex, int role);
    // 双击信号
    void signalDoubleClicked(const QModelIndex &iIndex);

   protected:
    virtual bool viewportEvent(QEvent *pEvent);

   private:
    StyledDelegate m_itemDelegate;
    StringListModel m_model;
};
