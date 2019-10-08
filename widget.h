#ifndef WIDGET_H
#define WIDGET_H

//#include <QStandardItemModel>
#include <QWidget>
#include <iostream>
#include <QSpinBox>
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
    void flushData();
    void uiInit();
    mb_cmu* mycmu;

   private:
    Ui::Widget* ui;
    MessageQueue* pmq;
    QTimer* timer;
    CMU_CONF config;
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
