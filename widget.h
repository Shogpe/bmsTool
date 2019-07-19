#ifndef WIDGET_H
#define WIDGET_H

#include <QDebug>
#include <QStandardItemModel>
#include <QWidget>
#include <iostream>

#include "mb_cmu.h"
using namespace std;
namespace Ui {
class Widget;
}

class Widget : public QWidget {
  Q_OBJECT

 public:
  explicit Widget(QWidget* parent = nullptr);
  ~Widget();
  void flushTemp();
  void flushVoltage();
  void flushStatus();
  mb_cmu* mycmu;

 private:
  Ui::Widget* ui;
  QTimer* timer;

 private slots:
  void timerUpDate();
};

#endif
