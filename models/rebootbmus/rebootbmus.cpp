#include "rebootbmus.h"
#include "ui_rebootbmus.h"
#include <QDebug>


Rebootbmus::Rebootbmus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Rebootbmus)
{
    ui->setupUi(this);
    connect(ui->pB_StartSend,&QPushButton::clicked,this,&Rebootbmus::btnclick);    
    connect(&timer_script,&QTimer::timeout,this,&Rebootbmus::timerUpadte);
    timer_script.setInterval(1000);
}

Rebootbmus::~Rebootbmus()
{
    delete ui;
}

void Rebootbmus::btnclick()
{
    auto btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;
    if(btn->objectName()=="pB_StartSend"){
        if(btn->text() == tr("开始发送")){
            btn->setText(tr("停止发送"));
            timer_script.start(ui->spinBox->value()*1000);
            Tx_Cnt = 0;
        }else if(btn->text() == tr("停止发送")){
            btn->setText(tr("开始发送"));
            timer_script.stop();
        }
    }
}

void Rebootbmus::timerUpadte()
{
    emit send_data();
    Tx_Cnt++;
    ui->label->setText(tr("TX:%1").arg(Tx_Cnt));
}

void Rebootbmus::closeEvent(QCloseEvent *event)
{
    emit CloseThisForm();
}
