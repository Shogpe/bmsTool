#include "protocolsetframe.h"
#include "ui_protocolsetframe.h"

ProtocolSetFrame::ProtocolSetFrame(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProtocolSetFrame)
{
    ui->setupUi(this);

}

ProtocolSetFrame::~ProtocolSetFrame()
{
    delete ui;
}


void ProtocolSetFrame::setText(QString txt)
{
    ui->str->setText(txt);
}

void ProtocolSetFrame::setCPVer(int ver)
{
    QString verText = "CMU_V";
    verText += QString::number(ver/1000);
    verText += QString("_%1").arg(ver%1000,3,10,QChar('0'));
    ui->currentCP->setText(verText);
}

void ProtocolSetFrame::setVerList(QList<int> list)
{
    cproVerList = list;

    ui->setCP->clear();

    foreach (int ver, cproVerList)
    {
        QString verText = "CMU_V";
        verText += QString::number(ver/1000);
        verText += QString("_%1").arg(ver%1000,3,10,QChar('0'));
        ui->setCP->addItem(verText);
    }

}

void ProtocolSetFrame::on_close_clicked()
{
    this->close();
}


void ProtocolSetFrame::on_runChange_clicked()
{
    int ver;
    QString verText = ui->setCP->currentText();
    verText.remove(" ");
    verText.remove("CMU_V");
    verText.remove("_");

    ver = verText.toInt();
    emit setCPVerConfirm(ver);
}

