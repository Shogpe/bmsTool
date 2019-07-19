#ifndef UIDEMO08_H
#define UIDEMO08_H

#include <QWidget>
#include "widget.h"
class QToolButton;

namespace Ui {
class MainUI;
}

class MainUI : public QWidget
{
    Q_OBJECT

public:
    explicit MainUI(QWidget *parent = 0);
    ~MainUI();

private:
    Ui::MainUI *ui;
    Widget *data_view;
    QList<int> pixCharMain;
    QList<QToolButton *> btnsMain;

    QList<int> pixCharConfig;
    QList<QToolButton *> btnsConfig;
    QTimer* timer;
private slots:
    void initForm();
    void buttonClick();
    void initLeftMain();
    void initLeftConfig();
    void leftMainClick();
    void leftConfigClick();
    void connectClick();
    void timerUpDate();

private slots:
    void on_btnMenu_Min_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Close_clicked();
};

#endif // UIDEMO08_H
