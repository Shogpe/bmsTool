#ifndef WIDGET_H
#define WIDGET_H

#include <QTableWidget>
#include <QWidget>
#include <iostream>
#include "mb_cmu.h"
#include "models/MyDoubleSpinBox/MyDoubleSpinBox.h"
#include "models/SOEModel/SOEModel.h"
#include "frmbalancebox.h"

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
    SOEModel m_model;
    frmBalanceBox *inputBalance = nullptr;
    bool eventFilter(QObject* obj, QEvent* event);
    int setValue(string name, double dval);
   private slots:
    void timerUpDate();
    void valueChange();
    void btn_released();
    void btn_contrl();
    void stateChanged();
    void checkChanged();
    void on_lineEditIP_editingFinished();
    void on_lineEditServIP_editingFinished();
    //

    void on_btnOutput_released();
    void on_btnInput_released();
};

#endif
