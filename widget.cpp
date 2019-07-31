#include "widget.h"

#include <QDateTime>
#include <QTimer>

#include "ui_widget.h"

Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpDate()));
    timer->start(2000);
    //
}

Widget::~Widget() {
    mycmu->stop = true;
    mycmu->wait();
    timer->stop();
    delete timer;
    delete mycmu;
    delete ui;
}

void Widget::uiInit(int NumOfBmu,int NumOfVol,int NumOfTemp,int NumOfStatus) {
//    int NumOfBmu = 20;
//    int NumOfVol = 12;
//    int NumOfTemp = 6;
//    int NumOfStatus = 5;

    ui->tableWidget->setColumnCount(NumOfVol);
    ui->tableWidget->setRowCount(NumOfBmu);
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QStringList hdr_list;
    for (int i = 0; i < ui->tableWidget->columnCount(); i++) {
        hdr_list.append(("voltage" + QString::number(i + 1)));
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    ui->tableWidget->setHorizontalHeaderLabels(hdr_list);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
    //
    ui->tableTemp->setColumnCount(NumOfTemp);
    ui->tableTemp->setRowCount(NumOfBmu);
    hdr_list.clear();
    for (int i = 0; i < ui->tableTemp->columnCount() - 2; i++) {
        hdr_list.append(("Tpack" + QString::number(i + 1)));
        ui->tableTemp->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    hdr_list.append(tr("Tp1"));
    hdr_list.append(tr("Tp2"));
    ui->tableTemp->setHorizontalHeaderLabels(hdr_list);

    ui->tableStatus->setColumnCount(NumOfStatus);
    ui->tableStatus->setRowCount(NumOfBmu);
    hdr_list.clear();
    hdr_list.append(tr("电压断线"));
    hdr_list.append(tr("温度断线"));
    hdr_list.append(tr("运行状态"));
    hdr_list.append(tr("故障状态"));
    hdr_list.append(tr("版本号"));
    ui->tableStatus->setHorizontalHeaderLabels(hdr_list);
}

void Widget::timerUpDate() {
    QTime t;
    t.start();  //将此时间设置为当前时间
    this->flushTemp();
    this->flushVoltage();
    this->flushStatus();
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
   // qDebug() << t.elapsed() << "ms";
}
void Widget::flushVoltage() {
    //一定要固定宽度，否则刷新很慢
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    for (int i = 0; i < mycmu->config.bmu_num; i++) {
        for (int j = 0; j < mycmu->config.vol_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = mycmu->tab_reg[i * mycmu->config.vol_num + j] / 10000.0f;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableWidget->setItem(i, j, item);
        }
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
void Widget::flushTemp() {
    ui->tableTemp->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableTemp->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int16_t* p = (int16_t*)&(mycmu->tab_reg[mycmu->config.bmu_num * mycmu->config.vol_num]);
    for (int i = 0; i < mycmu->config.bmu_num; i++) {
        for (int j = 0; j < mycmu->config.temp_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = *(p + i * mycmu->config.temp_num + j) / 10.0f;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableTemp->setItem(i, j, item);
        }
    }
    ui->tableTemp->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableTemp->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
void Widget::flushStatus() {
    ui->tableStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableStatus->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    uint16_t* p = (uint16_t*)&(
        mycmu->tab_reg[mycmu->config.bmu_num * mycmu->config.vol_num + mycmu->config.bmu_num * mycmu->config.temp_num]);
    for (int i = 0; i < mycmu->config.status_num; i++) {
        for (int j = 0; j < mycmu->config.bmu_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            uint16_t val = *(p + i * mycmu->config.bmu_num + j);
            item->setText(QString("0x%1").arg(int(val), 4, 16, QLatin1Char('0')));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableStatus->setItem(j, i, item);
        }
    }
    uint32_t* p32 = (uint32_t*)&(
        mycmu->tab_reg[mycmu->config.bmu_num * mycmu->config.vol_num + mycmu->config.bmu_num * mycmu->config.temp_num +
                       mycmu->config.bmu_num * mycmu->config.status_num]);
    // qDebug()<<mycmu->tab_reg[241];
    mycmu->cmu_ver = *(p32++);
    for (int j = 0; j < mycmu->config.bmu_num; j++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        uint32_t val = *(p32 + j);
        item->setText(QString("0x%1").arg(int(val), 8, 16, QLatin1Char('0')));
        //      item->setBackground(QBrush(QColor(Qt::lightGray)));
        //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableStatus->setItem(j, 4, item);
    }
    ui->tableStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableStatus->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
