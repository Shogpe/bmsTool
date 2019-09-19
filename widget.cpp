#include "widget.h"

#include <QDateTime>
#include <QSpinBox>
#include <QTimer>
#include "ui_widget.h"

void QAbstractSpinBox::wheelEvent(QWheelEvent *e) {}
QString QDoubleSpinBox::textFromValue(double value) const
{
    return QLocale().toString(value, 'g', QLocale::FloatingPointShortest);
}
Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpDate()));
    timer->start(2000);
    mycmu = nullptr;
    //
}

Widget::~Widget() {
    timer->stop();
    delete timer;
    delete ui;
}

void Widget::uiInit(int NumOfBmu, int NumOfVol, int NumOfTemp, int NumOfStatus) {
    ui->tableBMU->setColumnCount(NumOfVol + NumOfTemp + NumOfStatus);
    ui->tableBMU->setRowCount(NumOfBmu);
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QStringList hdr_list;
    for (int i = 0; i < NumOfVol; i++) {
        hdr_list.append(("Vol" + QString::number(i + 1)));
    }
    for (int i = 0; i < NumOfTemp - 2; i++) {
        hdr_list.append(("Tpack" + QString::number(i + 1)));
    }
    hdr_list.append(tr("Tp1"));
    hdr_list.append(tr("Tp2"));
    hdr_list.append(tr("电压断线"));
    hdr_list.append(tr("温度断线"));
    hdr_list.append(tr("运行状态"));
    hdr_list.append(tr("故障状态"));
    hdr_list.append(tr("版本号"));
    ui->tableBMU->setHorizontalHeaderLabels(hdr_list);
    ui->tableBMU->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableBMU->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        connect(dspbox, SIGNAL(valueChanged(double)), this, SLOT(valueChange()), Qt::UniqueConnection);
    }
}
void Widget::valueChange() {
    QDoubleSpinBox* b = (QDoubleSpinBox*)sender();
    QString name = b->objectName();
    if (name == "ClusterNB") {
        qDebug() << b->value();
    } else {
        qDebug() << name;
    }
}
void Widget::timerUpDate() {
    QTime t;
    t.start();  //将此时间设置为当前时间
    //
    uiInit(8, 16, 6, 5);
    this->flushData();
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
    qDebug() << t.elapsed() << "ms";
    ui->costTime->setValue(t.elapsed());
}
void Widget::flushData() {
    //一定要固定宽度，否则刷新很慢
    if (mycmu == nullptr) return;
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    int cloumn_offset = 0;
    uint16_t* pVol = (uint16_t*)&(mycmu->tab_reg[mycmu->tab_config[4].tab_offset]);
    for (int i = 0; i < mycmu->config.bmu_num; i++) {
        for (int j = 0; j < mycmu->config.vol_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = *(pVol + i * mycmu->config.vol_num + j) / 10000.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
        }
    }
    cloumn_offset += mycmu->config.vol_num;
    int16_t* pTemp = (int16_t*)&(mycmu->tab_reg[mycmu->tab_config[5].tab_offset]);
    for (int i = 0; i < mycmu->config.bmu_num; i++) {
        for (int j = 0; j < mycmu->config.temp_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = *(pTemp + i * mycmu->config.temp_num + j) / 10.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
        }
    }
    cloumn_offset += mycmu->config.temp_num;
    uint16_t* pStatus = (uint16_t*)&(mycmu->tab_reg[mycmu->tab_config[6].tab_offset]);
    for (int i = 0; i < mycmu->config.status_num; i++) {
        for (int j = 0; j < mycmu->config.bmu_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            uint16_t val = *(pStatus + i * mycmu->config.bmu_num + j);
            item->setText(QString("0x%1").arg(int(val), 4, 16, QLatin1Char('0')));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(j, i + cloumn_offset, item);
        }
    }
    cloumn_offset += mycmu->config.status_num;
    uint32_t* p32 = (uint32_t*)&(mycmu->tab_reg[mycmu->tab_config[7].tab_offset]);
    mycmu->cmu_ver = *(p32++);
    for (int j = 0; j < mycmu->config.bmu_num; j++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        uint32_t val = *(p32 + j);
        item->setText(QString("0x%1").arg(int(val), 8, 16, QLatin1Char('0')));
        //      item->setBackground(QBrush(QColor(Qt::lightGray)));
        //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(j, cloumn_offset, item);
    }
    //数据刷新完毕后自适应列宽
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
