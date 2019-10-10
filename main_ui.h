#ifndef UIDEMO08_H
#define UIDEMO08_H

#include <QWidget>
#include "qframelesswidget.h"
#include "widget.h"
#include "mb_cmu.h"

class QToolButton;

namespace Ui {
class MainUI;
}

class MainUI : public QFramelessWidget
{
    Q_OBJECT

public:
    explicit MainUI(QWidget *parent = nullptr);
    ~MainUI();

private:
    Ui::MainUI *ui;
    Widget *data_view;
    QList<int> pixCharMain;
    QList<QToolButton *> btnsMain;

    QList<int> pixCharConfig;
    QList<QToolButton *> btnsConfig;
    QTimer* timer;
    mb_cmu *pcmu;
    MessageQueue* pmq;
private slots:
    void initForm();
    void buttonClick();
    void valueChange();
    void initLeftMain();
    void initLeftConfig();
    void leftMainClick();
    void leftConfigClick();
    void btnClick();
    void timerUpDate();

private slots:
    void on_btnMenu_Min_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Close_clicked();
};

#endif // UIDEMO08_H
