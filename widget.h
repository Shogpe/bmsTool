#ifndef WIDGET_H
#define WIDGET_H

#include <QSpinBox>
#include <QWidget>
#include <QTableWidget>
#include <iostream>
#include "mb_cmu.h"

using namespace std;
#include<QDateTime>

class SOEModel : public QAbstractTableModel {
  QList<CMU_SOE> m_data;

 public:
  SOEModel(QObject* parent = {}) : QAbstractTableModel{parent} {}
  int rowCount(const QModelIndex&) const override { return m_data.count(); }
  int columnCount(const QModelIndex&) const override { return 6; }
  QVariant data(const QModelIndex& index, int role) const override {
    if (!index.isValid())
      return QVariant();

    if (role == Qt::TextAlignmentRole) {
      return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if (role == Qt::DisplayRole) {
      CMU_SOE index_soe = m_data.at(index.row());
      switch(index.column())
      {
        case 0:
          return QDateTime::fromMSecsSinceEpoch(index_soe.soe_time).toString("yyyy-MM-dd hh:mm:ss.zzz");
        case 1:
          return  QString("%1").arg(index_soe.soe_type);
        case 2:
          return QString("%1").arg(index_soe.soe_id);
        case 3:
          return QString("%1").arg(index_soe.soe_val);
        case 4:
          return QString("%1").arg(index_soe.soe_limit);
        case 5:
          return QString("%1").arg(index_soe.soe_stat);
      }
    }
    return QVariant();
  }
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
    switch (section) {
      case 0:
        return tr("时间");
      case 1:
        return tr("事件类型");
      case 2:
        return tr("事件ID");
      case 3:
        return tr("数据值");
      case 4:
        return tr("限值");
      case 5:
        return tr("系统状态");
      default:
        return {};
    }
  }
  void append(const CMU_SOE& soe) {
    beginInsertRows({}, m_data.count(), m_data.count());
    m_data.append(soe);
    endInsertRows();
  }
  bool setData(const QModelIndex &index, const CMU_SOE &soe, int role = Qt::EditRole)
  {
    if(!index.isValid())      return false;
    if(role == Qt::EditRole && index.row() <= m_data.size()) {
      m_data.replace(index.row(),soe);
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
   private slots:
    void timerUpDate();
    void valueChange();
    void on_btn_released();
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

#endif
