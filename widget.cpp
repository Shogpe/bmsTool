#include "widget.h"

#include <QDateTime>
#include <QTimer>
#include <QtDebug>
#include "ui_widget.h"

Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpDate()));
    timer->start(2000);
    mycmu = nullptr;
    pmq = MessageQueue::getInstance();
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
        connect(dspbox, &QDoubleSpinBox::editingFinished, this, &Widget::valueChange, Qt::UniqueConnection);
        // connect(dspbox, SIGNAL(valueChanged(double)), this, SLOT(valueChange(double)), Qt::UniqueConnection);
    }

    connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &Widget::on_dateTimeEdit_dateTimeChanged,
            Qt::UniqueConnection);
    connect(ui->btnUpgrade,&QPushButton::released,this,&Widget::on_btnUpgrade_released,Qt::UniqueConnection);
}
void Widget::valueChange() {
    QDoubleSpinBox* b = (QDoubleSpinBox*)sender();
    map<string, NodeReg>::iterator iter1;
    if (!b->hasFocus()) return;
    double dval = b->value();
    iter1 = mycmu->name_map.find(b->objectName().toStdString());
    if (iter1 != mycmu->name_map.end()) {
        qDebug() << iter1->second.index << ":" << iter1->first.c_str();
        try {
            uint16_t val[2] = {0};
            val[0] = iter1->second.index;
            qDebug() << b->value() << "," << iter1->second.factor << "," << b->value() / iter1->second.factor;
            //+0.5保障精度
            val[1] = static_cast<int>(dval / iter1->second.factor + 0.5 - (dval < 0));
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO;
            MsgCmd.data.resize(2 * sizeof(uint16_t));
            memcpy(MsgCmd.data.data(), &val, 2 * sizeof(uint16_t));
            pmq->sendMsg(0, MsgCmd);
            // dspbox->setValue(mycmu->tab_data.at(iter1->second).sysData.val.f32);
        } catch (exception& e) {
            qDebug() << e.what();
        }
    }
}
void Widget::timerUpDate() {
    QTime t;
    t.start();  //将此时间设置为当前时间
    //
    uiInit(8, 16, 6, 5);
    this->flushData();
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
    // qDebug() << t.elapsed() << "ms";
    ui->costTime->setValue(t.elapsed());
}
void Widget::flushData() {
    //一定要固定宽度，否则刷新很慢
    if (mycmu == nullptr) return;
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    memcpy(&config, &mycmu->config, sizeof(config));
    int cloumn_offset = 0;
    int data_index = 0;
    uint16_t* pVol = (uint16_t*)&(mycmu->tab_reg[data_index]);
    for (int i = 0; i < config.bmu_num; i++) {
        for (int j = 0; j < config.vol_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = *(pVol + i * mycmu->config.vol_num + j) / 10000.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
            data_index++;
        }
    }
    cloumn_offset += config.vol_num;
    int16_t* pTemp = (int16_t*)&(mycmu->tab_reg[data_index]);
    for (int i = 0; i < config.bmu_num; i++) {
        for (int j = 0; j < config.temp_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = *(pTemp + i * mycmu->config.temp_num + j) / 10.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
            data_index++;
        }
    }
    cloumn_offset += config.temp_num;
    uint16_t* pStatus = (uint16_t*)&(mycmu->tab_reg[data_index]);
    for (int i = 0; i < config.status_num; i++) {
        for (int j = 0; j < config.bmu_num; j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            uint16_t val = *(pStatus + i * config.bmu_num + j);
            item->setText(QString("0x%1").arg(int(val), 4, 16, QLatin1Char('0')));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(j, i + cloumn_offset, item);
            data_index++;
        }
    }
    cloumn_offset += config.status_num;
    uint32_t* p32 = (uint32_t*)&(mycmu->tab_reg[data_index]);
    mycmu->cmu_ver = *(p32++);
    for (int j = 0; j < config.bmu_num; j++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        uint32_t val = *(p32 + j);
        item->setText(QString("0x%1").arg(int(val), 8, 16, QLatin1Char('0')));
        //      item->setBackground(QBrush(QColor(Qt::lightGray)));
        //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(j, cloumn_offset, item);
        data_index++;
    }
    //数据刷新完毕后自适应列宽
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //刷新设置
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        map<string, NodeReg>::iterator iter1;
        iter1 = mycmu->name_map.find(dspbox->objectName().toStdString());
        if (iter1 != mycmu->name_map.end()) {
            // qDebug() << iter1->second << ":" << iter1->first.c_str();
            try {
                int index = iter1->second.index;
                if (dspbox->hasFocus()) continue;
                dspbox->setValue(mycmu->tab_data.at(index).sysData.val.f64);
            } catch (exception& e) {
                cout << e.what() << endl;
            }
        }
    }
    dspboxs = ui->tabCMU->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        map<string, NodeReg>::iterator iter1;
        iter1 = mycmu->name_map.find(dspbox->objectName().toStdString());
        if (iter1 != mycmu->name_map.end()) {
            // qDebug() << iter1->second << ":" << iter1->first.c_str();
            try {
                int index = iter1->second.index;
                //if (dspbox->hasFocus()) continue;
                dspbox->setValue(mycmu->tab_data.at(index).sysData.val.f64);
            } catch (exception& e) {
                cout << e.what() << endl;
            }
        }
    }
}

void Widget::on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime) {
    time_t time = dateTime.toTime_t();
    qDebug() << "now " << time;
    time /= 60 * 60;
    int day = (time + 8) % (24);
    ui->date_day->setValue(day);
}

void Widget::on_btnUpgrade_released() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_UPGRADE;
    uint16_t val = 0x5a78;
    MsgCmd.data.resize(sizeof(uint16_t));
    memcpy(MsgCmd.data.data(), &val, sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}
