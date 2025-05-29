#include "sysstatwd.h"
#include "ui_sysstatwd.h"
#include <QDebug>
#include <QLabel>

#define WIDTH_OTHER     20
#define UI_NUM_OF_ROW   7
SysStatWd::SysStatWd(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SysStatWd)
{
    ui->setupUi(this);

//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget->setFixedWidth(ui->tableWidget->columnWidth(0) + ui->tableWidget->verticalHeader()->width() + WIDTH_OTHER);

    statList.clear();
    labelUIList.clear();

    initAllStatUI();

    ui->tableWidget->hide();
    ui->line->hide();
}

SysStatWd::~SysStatWd()
{
    delete ui;
}

void SysStatWd::clear()
{
    for(int row = 1; row < ui->tableWidget->rowCount(); row++)
    {
        ui->tableWidget->removeRow(1);
    }
}

void SysStatWd::setVerList(QList<verLabel_T> list)
{
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int idx = 0;
    ui->tableWidget->setRowCount(list.count());
    foreach(verLabel_T v, list)
    {
        QTableWidgetItem* item = new QTableWidgetItem;
        QFont font = item->font();
        item->setText(v.ver);
        if (v.onLine) {
            item->setTextColor(QColor(Qt::darkGreen));
            font.setStrikeOut(false);
            font.setBold(false);
        } else {
            item->setTextColor(QColor(Qt::red));
            font.setStrikeOut(true);
        }
        item->setFont(font);
        //item->setToolTip(tr("Strikethrough indicates disconnection"));
        item->setToolTip(tr("删除线表示断线"));
        ui->tableWidget->setItem(idx,0,item);
        idx++;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    if(idx > 0)
    {
        ui->tableWidget->setFixedWidth(ui->tableWidget->columnWidth(0) + ui->tableWidget->verticalHeader()->width() + WIDTH_OTHER);
    }
}

void SysStatWd::initAllStatUI()
{
    foreach(QLabel* lb, labelUIList)
    {
        if(lb != NULL)
        {
            lb->setParent(NULL);
            delete lb;
        }
    }
    labelUIList.clear();

    if(ui->statWd != NULL)
    {
        QLayout* layout = ui->statWd->layout();

        if(layout != NULL)
        {
            delete layout;
        }
    }


    //从MAP里取出数据
    statList.clear();
    QMap<QString,QString>::iterator itor;
    for(itor = statMapDefault.begin(); itor != statMapDefault.end(); ++itor)
    {
        statLabel_T* stat = new statLabel_T;
        stat->objName = itor.key();
        stat->statName = itor.value();
        stat->flg = false;

        if(isExcepted(stat->objName))
        {
            //该协议需要排除掉这个ui
            continue;
        }
        statList.append(stat);
    }

    QGridLayout* lay = new QGridLayout(ui->statWd);
    lay->setHorizontalSpacing(6);
    lay->setVerticalSpacing(3);

    ui->statWd->setLayout(lay);
    int row = 0;
    int col = 0;

    foreach(statLabel_T* stat, statList)
    {
        QLabel* lb = new QLabel(ui->statWd);

        lb->setText(stat->statName);
        lb->setObjectName(stat->objName);
        lb->setAlignment(Qt::AlignCenter);
        lb->setFrameShape(QFrame::Box);
        lb->setLineWidth(1);
        QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Expanding);
        lb->setSizePolicy(sp);

        QString color = stat->flg ? TEXT_RED : TEXT_GREEN;
        lb->setStyleSheet(QString("%1").arg(color));

        lay->addWidget(lb,row,col,1,1);
        labelUIList.append(lb);

        col++;
        if(col >= UI_NUM_OF_ROW)
        {
            col = 0;
            row++;
        }
    }
}



bool SysStatWd::isExcepted(QString objName)
{
    foreach(QString var, exceptList[com_ver])
    {
        if(var == objName)
        {
            return true;
        }
    }
    return false;


}

void SysStatWd::setLabel(QString text, bool flag)
{
    foreach(statLabel_T* stat, statList)
    {

        if(stat->statName == text)
        {
            stat->flg = flag;
//            qDebug() <<">>>>>>>>>"<< stat->objName << text << stat->flg;
            break;
        }
    }
//    debugShowAllTrue();

}

void SysStatWd::debugShowAllTrue()
{
//    qDebug() << "<<<<<<<<<<<<<<<<true list:";
    foreach(statLabel_T* stat, statList)
    {
        if(stat->flg)
        {
            qDebug() << stat->statName;
        }
    }
//    qDebug() << "--------------------------------------";
}

void SysStatWd::refreashAllStat()
{
    foreach(QLabel* lb, labelUIList)
    {
        foreach(statLabel_T* stat, statList)
        {
            if(lb->objectName() == stat->objName)
            {
                QString color = stat->flg? "color:darkRed;text-decoration:underline;font:bold;" : "color:darkGreen;";
                lb->setStyleSheet(QString("%1").arg(color));
//                qDebug() <<"<<<<<<"<< stat->objName << stat->statName << lb->objectName() << stat->flg << "----" << color;
                break;
            }
        }
    }
}
