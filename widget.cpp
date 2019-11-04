#include "widget.h"
#include <QDateTime>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>

#include "myhelper.h"
#include "ui_widget.h"
Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    this->installEventFilter(this);
    this->timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::timerUpDate);
    timer->start(2000);
    mycmu = nullptr;
    pmq = MessageQueue::getInstance();
    pmq->registMsgQueue(99);
    config = {0, 0, 0, 0, 0};
    //
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        // connect(dspbox, &QDoubleSpinBox::editingFinished, this, &Widget::valueChange, Qt::UniqueConnection);
        connect(dspbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                &Widget::valueChange, Qt::UniqueConnection);
    }
    QList<QPushButton*> btns = ui->tabCtrl->findChildren<QPushButton*>();
    foreach (QPushButton* btn, btns) {
        connect(btn, &QPushButton::released, this, &Widget::btn_released, Qt::UniqueConnection);
    }
    QList<QCheckBox*> chkboxs = ui->G_FuncMask->findChildren<QCheckBox*>();
    foreach (QCheckBox* chkbox, chkboxs) {
        connect(chkbox, &QCheckBox::stateChanged, this, &Widget::stateChanged, Qt::UniqueConnection);
    }
    QList<QCheckBox*> RadioList;
    RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
              << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
    foreach (QCheckBox* rb, RadioList) {
        connect(rb, &QCheckBox::stateChanged, this, &Widget::checkChanged, Qt::UniqueConnection);
    }  //
    connect(ui->btnReadSOE, &QPushButton::released, this, &Widget::btn_contrl, Qt::UniqueConnection);

    ui->ViewSOE->verticalHeader()->hide();
    ui->ViewSOE->horizontalHeader()->setStretchLastSection(true);
    ui->ViewSOE->setModel(&m_model);
}

Widget::~Widget() {
    timer->stop();
    delete timer;
    delete ui;
}

void Widget::uiInit() {
    ui->tableBMU->setColumnCount(config.vol_num + config.T_num + config.Tp_num + config.status_num + 1);
    ui->tableBMU->setRowCount(config.bmu_num);
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QStringList hdr_list;
    for (int i = 0; i < config.vol_num; i++) {
        hdr_list.append(("Vol" + QString::number(i + 1)));
    }
    for (int i = 0; i < config.T_num; i++) {
        hdr_list.append(("Tpack" + QString::number(i + 1)));
    }
    for (int i = 0; i < config.Tp_num; i++) {
        hdr_list.append(("Tp" + QString::number(i + 1)));
    }
    hdr_list.append(tr("电压断线"));
    hdr_list.append(tr("温度断线"));
    hdr_list.append(tr("运行状态"));
    hdr_list.append(tr("故障状态"));
    hdr_list.append(tr("版本号"));
    ui->tableBMU->setHorizontalHeaderLabels(hdr_list);
    ui->tableBMU->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableBMU->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
}
void Widget::valueChange() {
    QDoubleSpinBox* b = (QDoubleSpinBox*)sender();
    double dval = b->value();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("要修改\"%1\"为 %2 ?")).arg(b->toolTip()).arg(dval)) !=
        QDialog::Accepted)
        return;
    this->setFocus();
    map<string, NodeReg>::iterator iter1;
    iter1 = mycmu->name_map.find(b->objectName().toStdString());
    if (iter1 != mycmu->name_map.end()) {
        try {
            uint16_t val[2] = {0};
            val[0] = iter1->second.index;
            //+0.5保障精度
            val[1] = static_cast<uint16_t>(dval / iter1->second.factor + 0.5 - (dval < 0));
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO;
            MsgCmd.data.resize(2 * sizeof(uint16_t));
            memcpy(MsgCmd.data.data(), &val, 2 * sizeof(uint16_t));
            pmq->sendMsg(0, MsgCmd);
        } catch (exception& e) {
            qDebug() << e.what();
        }
    }
}
void Widget::timerUpDate() {
    //    QTime t;
    //    t.start();  //将此时间设置为当前时间
    //
    TMsgData Msg;
    if (pmq->readMsg(99, Msg) != 0) {
        if (Msg.msg_type == 0) {
            memcpy(&config, Msg.data.data(), sizeof(config));
            Msg.data.clear();
            this->uiInit();
            qDebug() << QString("table:%1x%2")
                            .arg(config.bmu_num)
                            .arg(config.vol_num + config.T_num + config.Tp_num + config.status_num);
        } else if (Msg.msg_type == 1) {
            if (!mycmu) return;
            ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
            // qDebug() << "soe:" << mycmu->cmu_soe.new_soe_count << "," << mycmu->cmu_soe.soe_count;
            for (int i = 0; i < 500; i++) {
                // qDebug() << "apped " << i << "soe:" << mycmu->cmu_soe.list_soe[i].soe_time;
                QModelIndex index = m_model.index(i, 0, QModelIndex());
                // m_model.append({(uint64_t)QDateTime::currentDateTime().toMSecsSinceEpoch(), 1, 2, 3, 4, 5});
                if (!m_model.setData(index, mycmu->cmu_soe.list_soe[i])) {
                    m_model.append(mycmu->cmu_soe.list_soe[i]);
                }
            }
            ui->labelSOE->setText(
                QString("新SOE:%1,总计:%2").arg(mycmu->cmu_soe.new_soe_count).arg(mycmu->cmu_soe.soe_count));
            ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        }
    }
    this->flushData();
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
    // qDebug() << t.elapsed() << "ms";
}
void Widget::flushData() {
    //一定要固定宽度，否则刷新很慢
    if (mycmu == nullptr) return;
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    // memcpy(&config, &mycmu->config, sizeof(config));
    if (config.bmu_num > ui->tableBMU->rowCount()) return;
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
            // QTableWidgetItem* item = ui->tableBMU->item(i, j + cloumn_offset);
            // item->setText(QString("%1").arg(val, 0, 'g', 5));
            data_index++;
        }
    }
    cloumn_offset += config.vol_num;
    int16_t* pTemp = (int16_t*)&(mycmu->tab_reg[data_index]);
    for (int i = 0; i < config.bmu_num; i++) {
        for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = *(pTemp + i * (config.T_num + config.Tp_num) + j) / 10.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            //      item->setBackground(QBrush(QColor(Qt::lightGray)));
            //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
            data_index++;
        }
    }
    cloumn_offset += (config.T_num + config.Tp_num);
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
    uint32_t* p32 = reinterpret_cast<uint32_t*>(&(mycmu->tab_reg[data_index]));
    mycmu->cmu_ver = *(p32++);
    //版本号
    uint32_t comm_status1 = mycmu->tab_data.at(mycmu->name_map["sysComm1"].index).sysData.val.f64;
    uint32_t comm_status2 = mycmu->tab_data.at(mycmu->name_map["sysComm2"].index).sysData.val.f64;
    uint64_t comm_status = (comm_status2 << 32) | comm_status1;
    for (int j = 0; j < config.bmu_num; j++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        uint32_t val = *(p32 + j);
        item->setText(myHelper::IntegerToHexString(val));
        if (comm_status >> j & 0x01)
            item->setTextColor(QColor(Qt::darkGreen));
        else
            item->setTextColor(QColor(Qt::red));
        //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setFlags(item->flags() & (Qt::NoItemFlags));
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
            try {
                int index = iter1->second.index;
                if (dspbox->hasFocus()) continue;
                dspbox->blockSignals(true);
                dspbox->setValue(mycmu->tab_data.at(index).sysData.val.f64);
                dspbox->blockSignals(false);

            } catch (exception& e) {
                qDebug() << e.what();
            }
        } else {

        }
    }
    dspboxs = ui->tabCMU->findChildren<QDoubleSpinBox*>();
    dspboxs << ui->sysTime;
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        map<string, NodeReg>::iterator iter1;
        iter1 = mycmu->name_map.find(dspbox->objectName().toStdString());
        if (iter1 != mycmu->name_map.end()) {
            try {
                uint index = iter1->second.index;
                dspbox->setValue(mycmu->tab_data.at(index).sysData.val.f64);
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    map<string, NodeReg>::iterator iter1;
    iter1 = mycmu->name_map.find("sysStatus1");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_SysStatus->setTitle(QString("%1(%2)").arg(tr("系统状态")).arg(value));
        QList<QLabel*> SysStatus;
        SysStatus << ui->bSysErr << ui->bSysAlm << ui->bSysFull << ui->bSysEmpty << ui->bSysInit << ui->bSysCommErr
                  << ui->bSysBalance << ui->bSysCharge << ui->bSysDischarge << ui->bSysStop << ui->bSys10 << ui->bSys11
                  << ui->bSys12 << ui->bSys13 << ui->bSys14 << ui->bSys15;
        foreach (QLabel* Label, SysStatus) {
            try {
                QString color = (value >> SysStatus.indexOf(Label)) & 0x01 > 0 ? "red" : "green";
                Label->setStyleSheet(QString("color:%1").arg(color));
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("sysErrStatus");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_ErrStatus->setTitle(QString("%1(%2)").arg(tr("保护状态1")).arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bErr0 << ui->bErr1 << ui->bErr2 << ui->bErr3 << ui->bErr4 << ui->bErr5 << ui->bErr6
                   << ui->bErr7 << ui->bErr8 << ui->bErr9 << ui->bErr10 << ui->bErr11 << ui->bErr12 << ui->bErr13
                   << ui->bErr14 << ui->bErr15;
        foreach (QLabel* Label, StatusList) {
            try {
                QString color = (value >> StatusList.indexOf(Label)) & 0x01 > 0 ? "red" : "green";
                Label->setStyleSheet(QString("color:%1").arg(color));
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("sysErrStatus2");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_ErrStatus_2->setTitle(QString("%1(%2)").arg(tr("保护状态2")).arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bErr0_2 << ui->bErr1_2 << ui->bErr2_2 << ui->bErr3_2 << ui->bErr4_2 << ui->bErr5_2
                   << ui->bErr6_2 << ui->bErr7_2 << ui->bErr8_2 << ui->bErr9_2 << ui->bErr10_2 << ui->bErr11_2
                   << ui->bErr12_2 << ui->bErr13_2 << ui->bErr14_2 << ui->bErr15_2;
        foreach (QLabel* Label, StatusList) {
            try {
                QString color = (value >> StatusList.indexOf(Label)) & 0x01 > 0 ? "red" : "green";
                Label->setStyleSheet(QString("color:%1").arg(color));
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("sysAlmStatus");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_AlmStatus->setTitle(QString("%1(%2)").arg(tr("告警状态1")).arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bAlm0 << ui->bAlm1 << ui->bAlm2 << ui->bAlm3 << ui->bAlm4 << ui->bAlm5 << ui->bAlm6
                   << ui->bAlm7 << ui->bAlm8 << ui->bAlm9 << ui->bAlm10 << ui->bAlm11 << ui->bAlm12 << ui->bAlm13
                   << ui->bAlm14 << ui->bAlm15;
        foreach (QLabel* Label, StatusList) {
            try {
                QString color = (value >> StatusList.indexOf(Label)) & 0x01 > 0 ? "gold" : "green";
                Label->setStyleSheet(QString("color:%1").arg(color));
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("sysAlmStatus2");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_AlmStatus_2->setTitle(QString("%1(%2)").arg(tr("告警状态2")).arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bAlm0_2 << ui->bAlm1_2 << ui->bAlm2_2 << ui->bAlm3_2 << ui->bAlm4_2 << ui->bAlm5_2
                   << ui->bAlm6_2 << ui->bAlm7_2 << ui->bAlm8_2 << ui->bAlm9_2 << ui->bAlm10_2 << ui->bAlm11_2
                   << ui->bAlm12_2 << ui->bAlm13_2 << ui->bAlm14_2 << ui->bAlm15_2;
        foreach (QLabel* Label, StatusList) {
            try {
                QString color = (value >> StatusList.indexOf(Label)) & 0x01 > 0 ? "gold" : "green";
                Label->setStyleSheet(QString("color:%1").arg(color));
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("sysDIStatus");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_DIStatus->setTitle(QString("%1(%2)").arg(tr("DI状态")).arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bDI0 << ui->bDI1 << ui->bDI2 << ui->bDI3 << ui->bDI4 << ui->bDI5 << ui->bDI6 << ui->bDI7
                   << ui->bDI8 << ui->bDI9 << ui->bDI10 << ui->bDI11 << ui->bDI12 << ui->bDI13 << ui->bDI14
                   << ui->bDI15;
        foreach (QLabel* Label, StatusList) {
            try {
                QString color = (value >> StatusList.indexOf(Label)) & 0x01 > 0 ? "red" : "green";
                Label->setStyleSheet(QString("color:%1").arg(color));
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("sysDOStatus");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_DOStatus->setTitle(QString("%1(%2)").arg(tr("DO状态")).arg(value));
        QList<QCheckBox*> RadioList;
        RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
                  << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
        foreach (QCheckBox* rb, RadioList) {
            try {
                bool bit = (value >> RadioList.indexOf(rb)) & 0x01 > 0;
                QString color = bit ? "red" : "green";
                rb->setStyleSheet(QString("color:%1").arg(color));
                rb->blockSignals(true);
                rb->setChecked(bit);
                rb->blockSignals(false);
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("FuncMask");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_FuncMask->setTitle(QString("%1(%2)").arg(tr("使能位")).arg(value));
        QList<QCheckBox*> CheckBoxList;
        CheckBoxList << ui->bFunc0 << ui->bFunc1 << ui->bFunc2 << ui->bFunc3 << ui->bFunc4 << ui->bFunc5 << ui->bFunc6
                     << ui->bFunc7 << ui->bFunc8 << ui->bFunc9 << ui->bFunc10 << ui->bFunc11 << ui->bFunc12
                     << ui->bFunc13 << ui->bFunc14 << ui->bFunc15;
        foreach (QCheckBox* cb, CheckBoxList) {
            try {
                cb->blockSignals(true);
                cb->setChecked((value >> CheckBoxList.indexOf(cb)) & 0x01 > 0);
                cb->blockSignals(false);
            } catch (exception& e) {
                qDebug() << e.what();
            }
        }
    }
    if (!ui->lineEditServIP->hasFocus())
        ui->lineEditServIP->setText(myHelper::IPV4IntegerToString(mycmu->sys_para.Name.u32TftpServIP));
    if (!ui->lineEditIP->hasFocus())
        ui->lineEditIP->setText(myHelper::IPV4IntegerToString(mycmu->sys_para.Name.u32LocalIP));
    uint16_t id = mycmu->tab_data.at(mycmu->name_map["UmaxID"].index).sysData.val.f64;
    ui->UmaxID->setText(QString(tr("最大单体电压(%1)")).arg(myHelper::IDToString(id, config.vol_num)));
    id = mycmu->tab_data.at(mycmu->name_map["UminID"].index).sysData.val.f64;
    ui->UminID->setText(QString(tr("最小单体电压(%1)")).arg(myHelper::IDToString(id, config.vol_num)));
    id = mycmu->tab_data.at(mycmu->name_map["TmaxID"].index).sysData.val.f64;
    ui->TmaxID->setText(QString(tr("最高单体温度(%1)")).arg(myHelper::IDToString(id, config.T_num)));
    id = mycmu->tab_data.at(mycmu->name_map["TminID"].index).sysData.val.f64;
    ui->TminID->setText(QString(tr("最低单体温度(%1)")).arg(myHelper::IDToString(id, config.T_num)));
    id = mycmu->tab_data.at(mycmu->name_map["UmMaxID"].index).sysData.val.f64;
    ui->UmMaxID->setText(QString(tr("最大模组电压(%1)")).arg(myHelper::IDToString(id, config.vol_num)));
    id = mycmu->tab_data.at(mycmu->name_map["UdMaxID"].index).sysData.val.f64;
    ui->UdMaxID->setText(QString(tr("最大单体压差(%1)")).arg(myHelper::IDToString(id, config.vol_num)));
    id = mycmu->tab_data.at(mycmu->name_map["TpMaxID"].index).sysData.val.f64;
    ui->TpMaxID->setText(QString(tr("最大极柱温度(%1)")).arg(myHelper::IDToString(id, config.Tp_num)));
    id = mycmu->tab_data.at(mycmu->name_map["TrMaxID"].index).sysData.val.f64;
    ui->TrMaxID->setText(QString(tr("最大单体温升(%1)")).arg(myHelper::IDToString(id, config.T_num)));
}

static map<QString, int> btnMap = {{"btnDownBMS", CTRL_DOWN_BMS},
                                   {"btnDownBMSBoot", CTRL_DOWN_BMS_BTL},
                                   {"btnDownBMU", CTRL_DOWN_BMU},
                                   {"btnDownBMUBoot", CTRL_DOWN_BMU_BTL},
                                   {"btnUpBMU", CTRL_UPGRADE_BMU},
                                   {"btnBMULock", CTRL_CMD_BMU_LOCK},
                                   {"btnBMUUnlock", CTRL_CMD_BMU_UNLOCK},
                                   {"btnClearEng", CTRL_CMD_CLR_ENG},
                                   {"btnIFullAdj", CTRL_ADJ_I_FULL},
                                   {"btnIzeroAdj", CTRL_ADJ_I_ZERO},
                                   {"btnIleakFullAdj", CTRL_ADJ_ILEAK_FULL},
                                   {"btnIleakZeroAdj", CTRL_ADJ_ILEAK_ZERO},
                                   {"btnRFullAdj", CTRL_ADJ_RINS_FULL},
                                   {"btnRZeroAdj", CTRL_ADJ_RINS_ZERO},
                                   {"btnUfullAdj", CTRL_ADJ_U_FULL},
                                   {"btnUzeroAdj", CTRL_ADJ_U_ZERO},
                                   {"btnReboot", CTRL_CMD_REBOOT},
                                   {"btnTimeAdj", CERT_CMD_TIME_ADJ},
                                   {"btnResetDef", CTRL_CMD_RESET}};

void Widget::btn_released() {
    TMsgData MsgCmd;
    QPushButton* b = (QPushButton*)sender();
    QString name = b->objectName();
    map<QString, int>::iterator iter1;
    iter1 = btnMap.find(name);
    if (iter1 != btnMap.end()) {
        int type = iter1->second;
        MsgCmd.msg_type = type;
        pmq->sendMsg(0, MsgCmd);
    } else
        qDebug() << name;
}
void Widget::stateChanged() {
    QCheckBox* b = (QCheckBox*)sender();
    QList<QCheckBox*> CheckBoxList;
    uint16_t value[2] = {0};
    CheckBoxList << ui->bFunc0 << ui->bFunc1 << ui->bFunc2 << ui->bFunc3 << ui->bFunc4 << ui->bFunc5 << ui->bFunc6
                 << ui->bFunc7 << ui->bFunc8 << ui->bFunc9 << ui->bFunc10 << ui->bFunc11 << ui->bFunc12 << ui->bFunc13
                 << ui->bFunc14 << ui->bFunc15;
    foreach (QCheckBox* cb, CheckBoxList) { value[1] |= (cb->isChecked() << CheckBoxList.indexOf(cb)); }
    if (myHelper::ShowMessageBoxQuesion(
            QString(tr("确定%2\"%1\"吗").arg(b->text()).arg(b->isChecked() > 0 ? tr("开启") : tr("关闭")))) !=
        QDialog::Accepted)
        return;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO;
    value[0] = 98;
    MsgCmd.data.resize(2 * sizeof(uint16_t));
    memcpy(MsgCmd.data.data(), &value, 2 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}
void Widget::checkChanged() {
    QCheckBox* b = (QCheckBox*)sender();
    QList<QCheckBox*> RadioList;
    uint16_t value[2] = {0};

    RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
              << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
    value[0] = RadioList.indexOf(b) + 1;
    value[1] = b->isChecked();
    if (myHelper::ShowMessageBoxQuesion(
            QString(tr("确定%2\"%1\"吗").arg(b->text()).arg(b->isChecked() > 0 ? tr("-控合-") : tr("-控分-")))) !=
        QDialog::Accepted)
        return;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_DO;
    MsgCmd.data.resize(2 * sizeof(uint16_t));
    memcpy(MsgCmd.data.data(), &value, 2 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}
void Widget::btn_contrl() {
    TMsgData MsgCmd;
    QPushButton* b = (QPushButton*)sender();
    QString name = b->objectName();
    if (name == "btnReadSOE") {
        MsgCmd.msg_type = CERT_CMD_READ_SOE;
        MsgCmd.data.clear();
        pmq->sendMsg(0, MsgCmd);
        ui->labelSOE->setText(tr("读取中...请稍侯..."));
    } else
        qDebug() << name;
}

void Widget::on_lineEditIP_editingFinished() {
    QLineEdit* pEdit = ui->lineEditIP;
    if (!pEdit->isModified()) return;
    pEdit->setModified(false);
    if (!myHelper::IsIP(pEdit->text())) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        return;
    }
    this->setFocus();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要设备IP为%1吗").arg(pEdit->text()))) != QDialog::Accepted)
        return;
    uint32_t ip = myHelper::IPV4StringToInteger(pEdit->text());
    uint16_t val[3];
    val[0] = 99;
    ip = bswap_32(ip);
    val[1] = ip & 0xFFFF;
    val[2] = ip >> 16 & 0xFFFF;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO;
    MsgCmd.data.resize(3 * sizeof(uint16_t));
    memcpy(MsgCmd.data.data(), &val, 3 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}

void Widget::on_lineEditServIP_editingFinished() {
    QLineEdit* pEdit = ui->lineEditServIP;
    if (!pEdit->isModified()) return;
    pEdit->setModified(false);
    if (!myHelper::IsIP(pEdit->text())) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        return;
    }
    this->setFocus();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要修改服务器IP为%1吗").arg(pEdit->text()))) !=
        QDialog::Accepted)
        return;
    uint32_t ip = myHelper::IPV4StringToInteger(pEdit->text());
    uint16_t val[3];
    val[0] = 100;
    ip = bswap_32(ip);
    val[1] = ip & 0xFFFF;
    val[2] = ip >> 16 & 0xFFFF;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO;
    MsgCmd.data.resize(3 * sizeof(uint16_t));
    memcpy(MsgCmd.data.data(), &val, 3 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}

void Widget::on_btnOutput_released() {
    QString filename = QFileDialog::getSaveFileName(this, "Save", "", "*.xml");

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QDomDocument document;

    QString strHeader("version=\"1.0\" encoding=\"UTF-8\"");
    document.appendChild(document.createProcessingInstruction("xml", strHeader));

    QDomElement root_elem = document.createElement("configtemplate");
    root_elem.setAttribute("ver", 1);
    document.appendChild(root_elem);
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();

    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        QDomElement item1 = document.createElement("item");
        item1.setAttribute("name", dspbox->objectName());
        item1.setAttribute("value", dspbox->value());
        root_elem.appendChild(item1);
    }
    QTextStream out(&file);
    document.save(out, 4);
    file.close();
}

void Widget::on_btnInput_released() {
    QString filename = QFileDialog::getOpenFileName(this, "Open", "", "*.xml");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();
    QList<QString> name_list;
    name_list << "CellVolH"
              << "CellVolHH"
              << "CellVolL"
              << "CellVolLL"
              << "PackTH"
              << "PackTHH"
              << "PackTL"
              << "PackTLL"
              << "PackTdH"
              << "PackTdHH"
              << "PackTrH"
              << "PackTrHH"
              << "PoleTH"
              << "PoleTHH"
              << "ClusterCurH"
              << "ClusterCurHH"
              << "ClusterCurShort"
              << "ClusterVolH"
              << "ClusterVolHH"
              << "ClusterVolL"
              << "ClusterVolLL"
              << "ClusterRIns"
              << "ClusterCurLeak"
              << "ClusterTAlm"
              << "ClusterTErr"
              << "ClusterE"
              << "ClusterEAdj"
              << "ClusterEremain"
              << "ClusterIe"
              << "ClusterCurRange"
              << "ClusterILeakRg"
              << "ClusterVolRange"
              << "BalnceMask"
              << "BalnceStart"
              << "BalnceStartDiff"
              << "ClusterBmuNum"
              << "BmuCellNum"
              << "BmuPackTNum"
              << "BmuPoleTNum"
              << "ClusterAlmMask"
              << "ClusterErrMask"
              << "FuncMask";
    double d_value[42];
    uint16_t i_value[43];
    foreach (const QString cur_name, name_list) {
        uint index = mycmu->name_map[cur_name.toStdString()].index;
        d_value[name_list.indexOf(cur_name)] = mycmu->tab_data.at(index).sysData.val.f64;
    }

    QDomElement root = doc.documentElement();  //返回根节点
    QDomNode node = root.firstChild();         //获得第一个子节点
    while (!node.isNull())                     //如果节点不空
    {
        if (node.isElement())  //如果节点是元素
        {
            QDomElement e = node.toElement();  //转换为元素，注意元素和节点是两个数据结构，其实差不多
            int i = name_list.indexOf(e.attribute("name"));
            if (i != -1) d_value[i] = e.attribute("value").toDouble();
        }
        node = node.nextSibling();  //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    qDebug() << "load ok!";
    try {
        for (int i = 1; i < 43; i++) {
            i_value[i] =
                static_cast<uint16_t>(d_value[i - 1] / mycmu->name_map[name_list.at(i - 1).toStdString()].factor + 0.5 -
                                      (d_value[i - 1] < 0));
        }
    } catch (exception& e) {
        qDebug() << e.what();
    }
    qDebug() << "to Int ok!";
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO;
    MsgCmd.data.resize(43 * sizeof(uint16_t));
    i_value[0] = 57;
    memcpy(MsgCmd.data.data(), &i_value, 43 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}
//屏蔽本控件传递事件到父控件
bool Widget::eventFilter(QObject* obj, QEvent* event) {
    return true;  // QWidget::eventFilter(obj, event);
}
