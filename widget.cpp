#include "widget.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include "Toast.h"
#include "myhelper.h"
#include "socImporter.h"
#include "ui_widget.h"
Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    //    this->installEventFilter(this);
    this->timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::timerUpDate);
    mycmu = nullptr;
    pmq = MessageQueue::getInstance();
    pmq->registMsgQueue(99);
    initUpdateMenu();
    load_config();
    config = {0, 0, 0, 0, 0};
    //
    ui->cbProtocol->blockSignals(true);
    ui->cbProtocol->clear();
    ui->cbProtocol->addItem("CMU1.0", 0);
    ui->cbProtocol->addItem("CMU2.0", 1);
    ui->cbProtocol->addItem("CMU3.0", 2);
    ui->cbProtocol->addItem("CMU4.0", 3);
    //    ui->cbProtocol->addItem("CMU4.1", 4);
    ui->cbProtocol->addItem("CMU4.8", 5);
    ui->cbProtocol->blockSignals(false);
    QString protocol = QSettings("config.ini", QSettings::IniFormat).value("global/protocol", "CMU1.0").toString();
    for (int i = 0; i < ui->cbProtocol->count(); i++) {
        if (protocol == ui->cbProtocol->itemText(i)) {
            ui->cbProtocol->blockSignals(true);
            ui->cbProtocol->setCurrentIndex(i);
            ui->cbProtocol->blockSignals(false);
        }
    }
    this->mycmu = new mb_cmu((BMS_PROTOCOL)ui->cbProtocol->currentData().toUInt());
    connect(
        this->mycmu, static_cast<void (mb_cmu::*)(const QString&)>(&mb_cmu::signal_message), this,
        [this](const QString& msg) { Toast::showTip(msg, nullptr); }, Qt::UniqueConnection);
    mycmu->start();

    connect(ui->connectIP, &QLineEdit::editingFinished, this, &Widget::IpChange, Qt::UniqueConnection);
    connect(ui->tbtnConnect, SIGNAL(clicked(bool)), this, SLOT(btnClick()));
    //
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        connect(dspbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                &Widget::valueChange, Qt::UniqueConnection);
    }
    connect(ui->BalnceMask,
            static_cast<void (QDoubleSpinBox::*)(const QPoint& pos)>(&QDoubleSpinBox::customContextMenuRequested), this,
            [=](const QPoint& pos) {  // Handle global position
                QPoint globalPos = ui->BalnceMask->mapToGlobal(pos);
                // Create menu and insert some actions
                QMenu myMenu;
                myMenu.addAction(tr("修改"), this, [=]() {
                    int mode = ui->BalnceMask->value();
                    if (inputBalance == nullptr) {
                        inputBalance = new frmBalanceBox();
                        connect(inputBalance, &frmBalanceBox::valueChange, [this]() {
                            TMsgData MsgCmd;
                            uint16_t mode = inputBalance->getMode();
                            qDebug() << mode;
                            MsgCmd.msg_type = CTRL_AO_ADDR;
                            uint16_t value[2] = {5408, mode};
                            MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
                            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
                            MsgCmd.data.clear();
                            QByteArray b = inputBalance->getValue();
                            if (b.size() > 0) {
                                MsgCmd.msg_type = CTRL_AO_ADDR;
                                MsgCmd.data.append(b);
                                if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
                            }
                        });
                    }
                    inputBalance->setMode(mode);
                    inputBalance->open();
                    inputBalance->activateWindow();
                });
                // Show context menu at handling position
                myMenu.exec(globalPos);
            });
    connect(ui->BalnceStart,
            static_cast<void (QDoubleSpinBox::*)(const QPoint& pos)>(&QDoubleSpinBox::customContextMenuRequested), this,
            [=](const QPoint& pos) {  // Handle global position
                QPoint globalPos = ui->BalnceStart->mapToGlobal(pos);
                // Create menu and insert some actions
                QMenu myMenu;
                myMenu.addAction(tr("修改均衡配置"), this, [=]() {
                    int mode = ui->BalnceStart->value();
                    if (configBalance == nullptr) {
                        configBalance = new frmbalanceConfig();
                    }
                    configBalance->setValue(mode);
                    if (configBalance->exec() == QDialog::Accepted) {
                        TMsgData MsgCmd;
                        MsgCmd.msg_type = CTRL_AO_ADDR;
                        uint16_t value[2] = {5409, 0};
                        value[1] = configBalance->getValue();
                        MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
                        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
                    }
                });
                // Show context menu at handling position
                myMenu.exec(globalPos);
            });
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
    ui->ViewSOE->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ViewSOE, static_cast<void (QTableView::*)(const QPoint& pos)>(&QTableView::customContextMenuRequested),
            this,
            [=](const QPoint& pos) {  // Handle global position
                QPoint globalPos = ui->ViewSOE->mapToGlobal(pos);

                // Create menu and insert some actions
                QMenu myMenu;
                myMenu.addAction(tr("导出当前SOE"), this, [=]() {
                    QString fileName = QFileDialog::getSaveFileName(
                        this, tr("Save File"), tr("SOE导出") + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
                        tr("Excel(*.csv)"));
                    if (fileName.isEmpty()) return;
                    QFile file(fileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        QTextStream stream(&file);
                        stream << QChar(0xfeff);
                        int cc = m_model.columnCount();
                        QStringList list;
                        for (int i = 0; i < cc; i++) {
                            list << m_model.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                        }
                        stream << list.join(",") << endl;
                        for (int i = 0; i < m_model.rowCount(); i++) {
                            list.clear();
                            for (int j = 0; j < cc; j++) {
                                list << m_model.index(i, j).data().toString();
                            }
                            stream << list.join(",") << endl;
                        }
                        file.close();
                    }
                });
                // Show context menu at handling position
                myMenu.exec(globalPos);
            });
    //
    ui->tableBMU->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableBMU,
            static_cast<void (QTableWidget::*)(const QPoint& pos)>(&QTableWidget::customContextMenuRequested), this,
            [=](const QPoint& pos) {  // Handle global position
                QPoint globalPos = ui->tableBMU->mapToGlobal(pos);

                // Create menu and insert some actions
                QMenu myMenu;
                myMenu.addAction(tr("导出当前数据"), this, [=]() {
                    QTableWidget* table = ui->tableBMU;
                    QString fileName = QFileDialog::getSaveFileName(
                        this, tr("Save File"), tr("BMU数据") + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
                        tr("csv File(*.csv)"));
                    if (fileName.isNull()) {
                        return;
                    }
                    exportExecl(table, fileName);
                });
                // Show context menu at handling position
                myMenu.exec(globalPos);
            });
    ui->tableExtView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableExtView,
            static_cast<void (QTableWidget::*)(const QPoint& pos)>(&QTableWidget::customContextMenuRequested), this,
            [=](const QPoint& pos) {  // Handle global position
                QTableWidget* table = (QTableWidget*)sender();
                QPoint globalPos = table->mapToGlobal(pos);

                // Create menu and insert some actions
                QMenu myMenu;
                myMenu.addAction(tr("导出当前数据"), this, [=]() {
                    QString fileName = QFileDialog::getSaveFileName(
                        this, tr("Save File"),
                        tr("BMU扩展数据") + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
                        tr("csv File(*.csv)"));
                    if (fileName.isNull()) {
                        return;
                    }

                    exportExecl(table, fileName);
                });
                // Show context menu at handling position
                myMenu.exec(globalPos);
            });
    timer->start(500);
    Toast::showTip(tr("初始化完成"), nullptr);
}
bool Widget::exportExecl(QTableWidget* tableWidget, QString dirFile) {
    QFile file(dirFile);
    bool ret = file.open(QIODevice::Truncate | QIODevice::ReadWrite);
    if (!ret) {
        qDebug() << "open failure";
        return ret;
    }

    QTextStream stream(&file);
    stream << QChar(0xfeff);
    QString conTents;
    // 写入头
    QHeaderView* header = tableWidget->horizontalHeader();
    if (NULL != header) {
        for (int i = 0; i < header->count(); i++) {
            QTableWidgetItem* item = tableWidget->horizontalHeaderItem(i);
            if (NULL != item) {
                conTents += item->text() + ",";
            }
        }
        conTents += "\n";
    }

    // 写内容
    for (int row = 0; row < tableWidget->rowCount(); row++) {
        for (int column = 0; column < tableWidget->columnCount(); column++) {
            QTableWidgetItem* item = tableWidget->item(row, column);
            if (NULL != item) {
                QString str = item->text();

                str.replace(",", "|");
                conTents += str + ",";
            }
        }
        conTents += "\n";
    }
    stream << conTents;

    file.close();
    return true;
}
Widget::~Widget() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    pmq->sendMsg(0, MsgCmd);
    if (inputBalance) {
        inputBalance->close();
        inputBalance->deleteLater();
    }
    timer->stop();
    delete timer;
    delete ui;
}

void Widget::uiInit() {
    ui->tableBMU->setRowCount(config.bmu_num);
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QStringList hdr_list;

    if (this->mycmu->GetProtocalVer() < CMUV4) {
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
        ui->BalnceStart->blockSignals(true);
        ui->BalnceStart->setPrefix("均衡启动阈值(V) ");
        ui->BalnceStart->setSuffix("");
        ui->BalnceStart->setMaximum(6);
        ui->BalnceStart->setDecimals(4);
        ui->BalnceStart->setToolTip("均衡启动阈值");
        ui->BalnceStart->blockSignals(false);
        ui->BalnceStart->setContextMenuPolicy(Qt::NoContextMenu);

    } else if (this->mycmu->GetProtocalVer() >= CMUV4) {
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
        hdr_list.append(tr("母线电压(V)"));
        hdr_list.append(tr("均衡电流(A)"));
        hdr_list.append(tr("均衡故障"));
        hdr_list.append(tr("通道状态"));
        hdr_list.append(tr("均衡模式"));
        hdr_list.append(tr("CAN错误数"));
        hdr_list.append(tr("版本号"));
        // 特殊处理
        ui->BalnceStart->blockSignals(true);
        ui->BalnceStart->setPrefix("均衡配置 ");
        ui->BalnceStart->setSuffix("");
        ui->BalnceStart->setMaximum(100000);
        ui->BalnceStart->setDecimals(0);
        ui->BalnceStart->setToolTip("均衡配置");
        ui->BalnceStart->blockSignals(false);
        ui->BalnceStart->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    ui->tableBMU->setColumnCount(hdr_list.size());
    ui->tableBMU->setHorizontalHeaderLabels(hdr_list);
    ui->tableBMU->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableBMU->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个

    //定值显示和隐藏
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        dspbox->hide();
        map<string, NodeReg>::iterator iter1;
        iter1 = mycmu->name_map.find(dspbox->objectName().toStdString());
        if (iter1 != mycmu->name_map.end()) {
            try {
                dspbox->show();
            } catch (exception& e) {
                qWarning() << e.what();
            }
        }
    }
    // 扩展表格
    if (this->mycmu->GetProtocalVer() > CMUV3) {
        ui->DataWidget->setTabEnabled(ui->DataWidget->indexOf(ui->tabBalance), true);
        QStringList hdr_list2;
        for (int i = 0; i < config.vol_num; i++) {
            hdr_list2.append(("充电Ah" + QString::number(i + 1)));
            hdr_list2.append(("放电Ah" + QString::number(i + 1)));
        }
        ui->tableExtView->setRowCount(config.bmu_num);
        ui->tableExtView->setColumnCount(hdr_list2.size());
        ui->tableExtView->setHorizontalHeaderLabels(hdr_list2);
        ui->tableExtView->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
        ui->tableExtView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
    } else {
        ui->DataWidget->setTabEnabled(ui->DataWidget->indexOf(ui->tabBalance), false);
    }
}
int Widget::setValue(string name, double dval) {
    map<string, NodeReg>::iterator iter1;
    iter1 = mycmu->name_map.find(name);
    if (iter1 != mycmu->name_map.end()) {
        try {
            uint16_t val[2] = {0};
            val[0] = iter1->second.reg_addr;
            //+0.5保障精度
            val[1] = static_cast<uint16_t>(std::round(dval / iter1->second.factor));
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            MsgCmd.data.append(reinterpret_cast<char*>(&val), 2 * sizeof(uint16_t));
            pmq->sendMsg(0, MsgCmd);
        } catch (exception& e) {
            qWarning() << e.what();
        }
    } else {
        qDebug() << "can't find " << name.c_str();
    }
    return 0;
}
void Widget::valueChange() {
    QDoubleSpinBox* b = (QDoubleSpinBox*)sender();
    double dval = b->value();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("要修改\"%1\"为 %2 ?")).arg(b->toolTip()).arg(dval)) !=
        QDialog::Accepted) {
        return;
    }
    b->clearFocus();
    qDebug() << b->objectName() << ":" << dval;
    setValue(b->objectName().toStdString(), dval);
}

void Widget::timerUpDate() {
    QTime t;
    t.restart();  //将此时间设置为当前时间
    //
    if (mycmu == nullptr) return;
    if (this->mycmu->drv_status) {
        ui->labelStatus->setStyleSheet("color:green");
        ui->labelStatus->setText(tr("已连接"));
        if (this->mycmu->drv_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("软件过期，请更新！"));
        uint32_t val = this->mycmu->cmu_ver;
        ui->btnVer->setText(QString("版本号:%1").arg(myHelper::IntegerToHexString(val)));
        ui->tbtnConnect->setText("重连");
    } else {
        ui->labelStatus->setStyleSheet("color:red;text-decoration:underline;font:bold;");
        ui->labelStatus->setText(tr("未连接"));
        ui->tbtnConnect->setText("连接");
    }
    TMsgData Msg;
    while (pmq->readMsg(99, Msg) != 0) {
        if (Msg.msg_type == 0) {
            memcpy(&config, Msg.data.data(), sizeof(config));
            Msg.data.clear();
            this->uiInit();
        } else if (Msg.msg_type == 1) {
            if (!mycmu) return;
            if (Msg.data.toInt() < 0) {
                ui->labelSOE->setText(tr("读取失败!!!"));
                break;
            }
            ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
            // qDebug() << "soe:" << mycmu->cmu_soe.new_soe_count << "," << mycmu->cmu_soe.soe_count;
            //            for (int i = 0; i < 500; i++) {
            //                // qDebug() << "apped " << i << "soe:" << mycmu->cmu_soe.list_soe[i].soe_time;
            //                QModelIndex index = m_model.index(i, 0, QModelIndex());
            //                // m_model.append({(uint64_t)QDateTime::currentDateTime().toMSecsSinceEpoch(), 1, 2, 3, 4,
            //                5}); if (!m_model.setData(index, mycmu->cmu_soe.list_soe[i])) {
            //                    m_model.append(mycmu->cmu_soe.list_soe[i]);
            //                }
            //            }
            uint32_t version = mycmu->cmu_ver & 0x00FFFFFF;
            qDebug() << QString::number(version, 16);
            if (((ui->cbProtocol->currentText().contains("CMU4")) && (version >= 0x00000402)) ||
                ((!ui->cbProtocol->currentText().contains("CMU4")) && (version >= 0x00000407))) {
                m_model.setData(mycmu->cmu_soe.list_soe, 500, db_manager::SOE_BMS2);
            } else {
                m_model.setData(mycmu->cmu_soe.list_soe, 500, db_manager::SOE_BMS1);
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
    uint32_t comm_status1 = mycmu->tab_data.at(mycmu->name_map["sysComm1"].index).sysData.val.f64;
    uint32_t comm_status2 = mycmu->tab_data.at(mycmu->name_map["sysComm2"].index).sysData.val.f64;
    uint64_t comm_status = ((uint64_t)comm_status2 << 32) | comm_status1;
    ui->CommStatus->setText(QString(tr("通信状态: %1")).arg(comm_status));

    QTableWidgetItem* item;
    for (int i = 0; i < config.bmu_num; i++) {
        cloumn_offset = 0;
        for (int j = 0; j < config.vol_num; j++) {
            item = new QTableWidgetItem();
            double val = this->mycmu->bmu_data[i].Ucell[j] / 10000.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
        }

        cloumn_offset += config.vol_num;

        for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = this->mycmu->bmu_data[i].Tcell[j] / 10.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
        }

        cloumn_offset += (config.T_num + config.Tp_num);

        // 电压断线，温度断线，运行状态，故障状态
        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].Ubreak, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].Tbreak, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].RunStat, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].ErrStat, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(i, cloumn_offset++, item);

        if (mycmu->GetProtocalVer() > CMUV3) {
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(mycmu->bmu_data[i].BalU24 / 1000.0));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(mycmu->bmu_data[i].BalIdc / 1000.0));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(mycmu->GetBalanceStatus(mycmu->bmu_data[i].BalErr));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(mycmu->GetBalanceStatus(mycmu->bmu_data[i].BalStat));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(mycmu->GetBalanceValue(mycmu->bmu_data[i].BalMode));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            // CAN通信错误计数
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(this->mycmu->bmu_data[i].CanErr));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
        }
        //版本号
        item = new QTableWidgetItem();
        item->setText(myHelper::IntegerToHexString(mycmu->bmu_data[i].Version));
        if (comm_status >> i & 0x01) {
            item->setTextColor(QColor(Qt::darkGreen));
            QFont font = item->font();
            font.setUnderline(false);
            font.setBold(false);
            item->setFont(font);
        } else {
            item->setTextColor(QColor(Qt::red));
            QFont font = item->font();
            font.setUnderline(true);
            font.setBold(true);
            item->setFont(font);
        }
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(i, cloumn_offset++, item);
    }
    //数据刷新完毕后自适应列宽
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //刷新定值
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
                qWarning() << e.what();
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
            dspbox->show();
            try {
                uint index = iter1->second.index;
                dspbox->setValue(mycmu->tab_data.at(index).sysData.val.f64);
            } catch (exception& e) {
                qWarning() << e.what();
            }
        } else {
            dspbox->hide();
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
        QStringList textList;
        textList << tr("总故障") << tr("总告警") << tr("充满") << tr("放空") << tr("未初始化") << tr("通信故障")
                 << tr("均衡") << tr("充电") << tr("放电") << tr("停机") << tr("升级") << tr("绝缘通信") << tr("自检")
                 << tr("BMU拨码") << tr("BMU故障") << tr("并网");
        foreach (QLabel* Label, SysStatus) {
            try {
                QString color = ((value >> SysStatus.indexOf(Label)) & 0x01) > 0
                                    ? "color:red;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
                Label->setText(textList.at(SysStatus.indexOf(Label)));
            } catch (exception& e) {
                qWarning() << e.what();
            }
        }
    }
    iter1 = mycmu->name_map.find("BootVer");
    if (iter1 != mycmu->name_map.end()) {
        ui->BootVer->show();
        uint32_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->BootVer->setText(QString(tr("Boot版本: %1")).arg(myHelper::IntegerToHexString(value)));
    } else {
        ui->BootVer->hide();
    }
    iter1 = mycmu->name_map.find("InsVer");
    if (iter1 != mycmu->name_map.end()) {
        ui->InsVer->show();
        uint32_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->InsVer->setText(QString(tr("绝缘版本: %1")).arg(myHelper::IntegerToHexString(value)));
    } else {
        ui->InsVer->hide();
    }

    iter1 = mycmu->name_map.find("sysStatus2");
    if (iter1 != mycmu->name_map.end()) {
        uint16_t value = mycmu->tab_data.at(iter1->second.index).sysData.val.f64;
        ui->G_SysStatus_2->setTitle(QString("%1(%2)").arg(tr("系统状态2")).arg(value));
        QList<QLabel*> SysStatus;
        SysStatus << ui->bSysErr_2 << ui->bSysAlm_2 << ui->bSysFull_2 << ui->bSysEmpty_2 << ui->bSysInit_2
                  << ui->bSysCommErr_2 << ui->bSysBalance_2 << ui->bSysCharge_2 << ui->bSysDischarge_2 << ui->bSysStop_2
                  << ui->bSys10_2 << ui->bSys11_2 << ui->bSys12_2 << ui->bSys13_2 << ui->bSys14_2 << ui->bSys15_2;
        QStringList textList;
        textList << tr("IO解锁") << tr("绝缘使能") << tr("") << tr("") << tr("") << tr("") << tr("") << tr("") << tr("")
                 << tr("") << tr("") << tr("") << tr("") << tr("") << tr("") << tr("");
        foreach (QLabel* Label, SysStatus) {
            try {
                QString color = ((value >> SysStatus.indexOf(Label)) & 0x01) > 0
                                    ? "color:red;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
                Label->setText(textList.at(SysStatus.indexOf(Label)));
                //                if (SysStatus.indexOf(Label) > 2) {
                //                    Label->setHidden(true);
                //                }
            } catch (exception& e) {
                qWarning() << e.what();
            }
        }
    } else {
        ui->G_SysStatus_2->setHidden(true);
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
                QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                    ? "color:red;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
            } catch (exception& e) {
                qWarning() << e.what();
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
                QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                    ? "color:red;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
            } catch (exception& e) {
                qWarning() << e.what();
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
                QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                    ? "color:gold;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
            } catch (exception& e) {
                qWarning() << e.what();
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
                QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                    ? "color:gold;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
            } catch (exception& e) {
                qWarning() << e.what();
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
        QStringList textList;
        textList << tr("QF状态") << tr("KM+状态") << tr("KM-状态") << tr("KMR状态") << tr("故障输入") << tr("主从状态")
                 << tr("备用7") << tr("备用8") << tr("备用9") << tr("备用10") << tr("备用11") << tr("备用12")
                 << tr("备用13") << tr("备用14") << tr("备用15") << tr("备用16");
        foreach (QLabel* Label, StatusList) {
            try {
                QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                    ? "color:red;text-decoration:underline;font:bold;"
                                    : "color:green";
                Label->setStyleSheet(QString("%1").arg(color));
                Label->setText(textList.at(StatusList.indexOf(Label)));
            } catch (exception& e) {
                qWarning() << e.what();
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
        QStringList textList;
        textList << tr("QF输出") << tr("KM+输出") << tr("KM-输出") << tr("KMR输出") << tr("故障输出") << tr("充电指示")
                 << tr("放电指示") << tr("系统运行") << tr("BMU供电") << tr("告警输出") << tr("风扇开启")
                 << tr("备用12") << tr("备用13") << tr("备用14") << tr("备用15") << tr("备用16");
        foreach (QCheckBox* rb, RadioList) {
            try {
                bool bit = ((value >> RadioList.indexOf(rb)) & 0x01) > 0;
                QString color = bit ? "color:red;text-decoration:underline;font:bold;" : "color:green";
                rb->setStyleSheet(QString("%1").arg(color));
                rb->blockSignals(true);
                rb->setChecked(bit);
                rb->blockSignals(false);
                rb->setText(textList.at(RadioList.indexOf(rb)));

            } catch (exception& e) {
                qWarning() << e.what();
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
                cb->setChecked(((value >> CheckBoxList.indexOf(cb)) & 0x01) > 0);
                cb->blockSignals(false);
            } catch (exception& e) {
                qWarning() << e.what();
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
    if (ui->DataWidget->currentWidget()->objectName() == tr("tabBalance")) {
        QTableWidgetItem* item;
        int offset = 0;
        ui->tableExtView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->tableExtView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        for (int i = 0; i < config.bmu_num; i++) {
            offset = 0;
            double val = 0;
            for (int j = 0; j < config.vol_num; j++) {
                item = new QTableWidgetItem();
                val = this->mycmu->bmu_data[i].BalChgAh[j];
                item->setText(QString("%1").arg(val, 0, 'g', 5));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);
                item = new QTableWidgetItem();
                val = this->mycmu->bmu_data[i].BalDischgAh[j];
                item->setText(QString("%1").arg(val, 0, 'g', 5));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);
            }
        }
        ui->tableExtView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableExtView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}
struct mb_cmd {
    uint16_t type;
    uint16_t addr;
    uint16_t value;
};
static map<QString, mb_cmd> btnMap = {{"btnBMULock", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_BMU_LOCK}},
                                      {"btnBMUUnlock", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_BMU_UNLOCK}},
                                      {"btnClearEng", {CTRL_AO_ADDR, ADDR_CLEAR_ENG, MB_CLEAR_ENG}},
                                      {"btnUploadTrig", {CTRL_AO_ADDR, ADDR_CLEAR_ENG, MB_UPLOAD_Trig}},
                                      {"btnIFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_IFull}},
                                      {"btnIBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_IBase}},
                                      {"btnIZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_IZero}},
                                      {"btnIleakFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_LFull}},
                                      {"btnIleakBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_LBase}},
                                      {"btnIleakZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_LZero}},
                                      {"btnRFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_TFull}},  //预留
                                      {"btnRBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_TBase}},
                                      {"btnRZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_TZero}},
                                      {"btnUFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_VFull}},
                                      {"btnUBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_VBase}},
                                      {"btnUZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_VZero}},
                                      {"btnReboot", {CTRL_CMD_REBOOT, ADDR_REBOOT, MB_REBOOT}},
                                      {"btnIOunlock", {CTRL_AO_ADDR, ADDR_IO_EN, MB_IO_UNLOCK}},
                                      {"btnIOlock", {CTRL_AO_ADDR, ADDR_IO_EN, MB_IO_LOCK}},
                                      {"btnAutoKMON", {CTRL_AO_ADDR, ADDR_CTRL_AUTO, MB_CTRL_ON}},
                                      {"btnAutoKMOFF", {CTRL_AO_ADDR, ADDR_CTRL_AUTO, MB_CTRL_OFF}},
                                      {"btnKMRON", {CTRL_AO_ADDR, ADDR_CTRL_KMR, MB_CTRL_ON}},
                                      {"btnKMROFF", {CTRL_AO_ADDR, ADDR_CTRL_KMR, MB_CTRL_OFF}},
                                      {"btnQFON", {CTRL_AO_ADDR, ADDR_CTRL_QF, MB_CTRL_ON}},
                                      {"btnQFOFF", {CTRL_AO_ADDR, ADDR_CTRL_QF, MB_CTRL_OFF}},
                                      {"btnKMPON", {CTRL_AO_ADDR, ADDR_CTRL_KMP, MB_CTRL_ON}},
                                      {"btnKMPOFF", {CTRL_AO_ADDR, ADDR_CTRL_KMP, MB_CTRL_OFF}},
                                      {"btnKMNON", {CTRL_AO_ADDR, ADDR_CTRL_KMN, MB_CTRL_ON}},
                                      {"btnKMNOFF", {CTRL_AO_ADDR, ADDR_CTRL_KMN, MB_CTRL_OFF}},
                                      {"btnFanON", {CTRL_AO_ADDR, ADDR_CTRL_FAN, MB_CTRL_ON}},
                                      {"btnFanOFF", {CTRL_AO_ADDR, ADDR_CTRL_FAN, MB_CTRL_OFF}},
                                      {"btnAcON", {CTRL_AO_ADDR, ADDR_CTRL_AC, MB_CTRL_ON}},
                                      {"btnAcOFF", {CTRL_AO_ADDR, ADDR_CTRL_AC, MB_CTRL_OFF}},
                                      {"btnResON", {CTRL_AO_ADDR, ADDR_CTRL_RES, MB_CTRL_ON}},
                                      {"btnResOFF", {CTRL_AO_ADDR, ADDR_CTRL_RES, MB_CTRL_OFF}},
                                      {"btnTimeAdj", {CERT_CMD_TIME_ADJ, 0, 0}},
                                      {"btnResetDef", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_FACTORY}}};

void Widget::btn_released() {
    TMsgData MsgCmd;
    uint16_t val[3];
    QPushButton* b = reinterpret_cast<QPushButton*>(sender());
    QString name = b->objectName();
    map<QString, mb_cmd>::iterator iter1;
    iter1 = btnMap.find(name);
    if (iter1 != btnMap.end()) {
        mb_cmd cmd = iter1->second;
        MsgCmd.msg_type = cmd.type;
        MsgCmd.data.append(reinterpret_cast<char*>(&cmd.addr), sizeof(uint16_t));
        MsgCmd.data.append(reinterpret_cast<char*>(&cmd.value), sizeof(uint16_t));
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
    } else if (name == "btnRUAdj") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = ADDR_RINS_ADJ;
        val[1] = MB_RU_ADJ;
        bool lbok;
        QString value = myHelper::showInputBox("绝缘电压校准值:", lbok);
        if (lbok) {
            val[2] = value.toDouble(&lbok) * 10;
            if (lbok) {
                qDebug() << "Adj:" << val[2];
                MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);

    } else if (name == "btnRpAdj") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = ADDR_RINS_ADJ;
        val[1] = MB_RP_ADJ;
        bool lbok;
        QString value = myHelper::showInputBox("正绝缘电阻校准值:", lbok);
        if (lbok) {
            val[2] = value.toDouble(&lbok) * 10;
            if (lbok) {
                qDebug() << "Adj:" << val[2];
                MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);

    } else if (name == "btnRnAdj") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = ADDR_RINS_ADJ;
        val[1] = MB_RN_ADJ;
        bool lbok;
        QString value = myHelper::showInputBox("负绝缘电阻校准值:", lbok);
        if (lbok) {
            val[2] = value.toDouble(&lbok) * 10;
            if (lbok) {
                qDebug() << "Adj:" << val[2];
                MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
    } else if (name == "btnBalance") {
        uint8_t mode = 0;
        map<string, NodeReg>::iterator iter1;
        iter1 = mycmu->name_map.find("BalnceMask");
        if (iter1 != mycmu->name_map.end()) {
            try {
                int index = iter1->second.index;
                mode = mycmu->tab_data.at(index).sysData.val.f64;
            } catch (exception& e) {
                qWarning() << e.what();
            }
        } else {
        }
        qDebug() << mode;
        //        bool lbok;
        if (inputBalance == nullptr) {
            inputBalance = new frmBalanceBox();
            connect(inputBalance, &frmBalanceBox::valueChange, [this]() {
                TMsgData MsgCmd;
                uint16_t mode = inputBalance->getMode();
                qDebug() << mode;
                MsgCmd.msg_type = CTRL_AO_ADDR;
                uint16_t value[2] = {5408, mode};
                MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
                if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
                MsgCmd.data.clear();
                QByteArray ba = inputBalance->getValue();
                if (ba.size() > 0) {
                    MsgCmd.msg_type = CTRL_AO_ADDR;
                    MsgCmd.data.append(ba);
                    if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
                }
            });
        }
        inputBalance->setMode(mode);
        inputBalance->open();
        inputBalance->activateWindow();
    } else if (name == "btnImportSOC") {
        QByteArray ba = SOCImport();
        //        if ((sizeof(uint16_t) * 101) != b.size()) {
        //            myHelper::ShowMessageBoxInfo("数据长度不合法！");
        //            return;
        //        }
        if (0 == ba.size()) {
            return;
        }
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_AO_ADDR;
        uint16_t value = 4096;
        MsgCmd.data.append(reinterpret_cast<char*>(&value), sizeof(uint16_t));
        MsgCmd.data.append(ba);
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
    } else if (name == "btnRCtrl") {
        bool ok = false;
        QStringList items;
        items << tr("全程投入") << tr("远程投入") << tr("远程断开");
        QString text =
            QInputDialog::getItem(this, tr("绝缘检测控制"), tr("请输入绝缘检测控制方式："), items, 0, false, &ok);
        qDebug() << ok << text;
        if (ok) {
            uint16_t mode = 0x0;
            if (text == tr("远程投入")) {
                mode = 0xAA55;
            } else if (text == tr("远程断开")) {
                mode = 0x55AA;
            }
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t value[2] = {65287, mode};
            MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        }
    } else if (name == "btnRClrErr") {
        if (myHelper::ShowMessageBoxQuesion(tr("是否清除绝缘检测故障？")) == QDialog::Accepted) {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t value[2] = {65288, 0xAA55};
            MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        }
    } else if (name == "btnSetSOC") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = 0xFFF6;
        bool lbok;
        QString value = myHelper::showInputBox("SOC标定值:", lbok);
        if (lbok) {
            val[1] = (uint16_t)(value.toDouble(&lbok) * 10) | 0x5000;
            if (lbok && (value.toDouble() <= 100) && (value.toDouble() >= 0)) {
                qDebug() << "Adj:" << val[1];
                MsgCmd.data.append(reinterpret_cast<char*>(&val), 2 * sizeof(val[0]));
            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
    } else if (name == "btnSetSOH") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = 0xFFF6;
        bool lbok;
        QString value = myHelper::showInputBox("SOH标定值:", lbok);
        if (lbok) {
            val[1] = (uint16_t)(value.toDouble(&lbok) * 10) | 0xA000;
            if (lbok && (value.toDouble() <= 100) && (value.toDouble() >= 0)) {
                qDebug() << "Adj:" << val[1];
                MsgCmd.data.append(reinterpret_cast<char*>(&val), 2 * sizeof(val[0]));
            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
    } else if (name == "btnAdjSOC") {
        if (myHelper::ShowMessageBoxQuesion(tr("是否校准SOC？")) == QDialog::Accepted) {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t value[2] = {0xFFF5, 0x1EA5};
            MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        }
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
    MsgCmd.msg_type = CTRL_AO_ADDR;
    value[0] = mycmu->name_map["FuncMask"].reg_addr;
    MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}
void Widget::checkChanged() {
    QCheckBox* b = (QCheckBox*)sender();
    QList<QCheckBox*> RadioList;
    uint16_t value[2] = {0};
    RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
              << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
    value[0] = RadioList.indexOf(b) + 1;
#if 0
    value[1] = b->isChecked();
    if (myHelper::ShowMessageBoxQuesion(
                QString(tr("确定%2\"%1\"吗").arg(b->text()).arg(b->isChecked() > 0 ? tr("-控合-") : tr("-控分-")))) !=
            QDialog::Accepted){
        return;
    }
#else
    QMessageBox box(QMessageBox::Warning, "输出控制", QString("当前控制出口为：%1").arg(b->text()));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    box.setButtonText(QMessageBox::Yes, QString("控 合"));
    box.setButtonText(QMessageBox::No, QString("控 分"));
    box.setButtonText(QMessageBox::Cancel, QString("取 消"));
    int ret = box.exec();
    switch (ret) {
        case QMessageBox::Yes:
            value[1] = true;
            break;
        case QMessageBox::No:
            value[1] = false;
            break;
        default:
            return;
    }
#endif
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_DO;
    MsgCmd.data.append((char*)&value, 2 * sizeof(uint16_t));
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
    QString ip_str = pEdit->text();
    if (!pEdit->isModified()) return;
    pEdit->setModified(false);
    if (!myHelper::IsIP(ip_str)) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        pEdit->undo();
        return;
    }
    this->setFocus();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要设备IP为%1吗").arg(ip_str))) != QDialog::Accepted) {
        pEdit->undo();
        return;
    }
    uint32_t ip = myHelper::IPV4StringToInteger(ip_str);
    uint16_t val[3];
    val[0] = mycmu->name_map["IP"].reg_addr;
    ip = bswap_32(ip);
    val[1] = ip & 0xFFFF;
    val[2] = ip >> 16 & 0xFFFF;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO_ADDR;
    MsgCmd.data.append((char*)&val, 3 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}

void Widget::on_lineEditServIP_editingFinished() {
    QLineEdit* pEdit = ui->lineEditServIP;
    if (!pEdit->isModified()) return;
    QString ip_str = pEdit->text();
    pEdit->setModified(false);
    if (!myHelper::IsIP(ip_str)) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        pEdit->undo();
        return;
    }
    this->setFocus();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要修改服务器IP为%1吗").arg(ip_str))) != QDialog::Accepted) {
        pEdit->undo();
        return;
    }
    uint32_t ip = myHelper::IPV4StringToInteger(ip_str);
    uint16_t val[3];
    val[0] = mycmu->name_map["ServerIP"].reg_addr;
    ip = bswap_32(ip);
    val[1] = ip & 0xFFFF;
    val[2] = ip >> 16 & 0xFFFF;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO_ADDR;
    MsgCmd.data.append((char*)&val, 3 * sizeof(uint16_t));
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
        if (!dspbox->isHidden()) {
            QDomElement item1 = document.createElement("item");
            item1.setAttribute("name", dspbox->objectName());
            item1.setAttribute("name_cn", dspbox->toolTip());
            item1.setAttribute("value", dspbox->value());
            root_elem.appendChild(item1);
        }
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
    QDomElement root = doc.documentElement();  //返回根节点
    QDomNode node = root.firstChild();         //获得第一个子节点
    while (!node.isNull())                     //如果节点不空
    {
        if (node.isElement())  //如果节点是元素
        {
            QDomElement e = node.toElement();  //转换为元素，注意元素和节点是两个数据结构，其实差不多
            QDoubleSpinBox* dspbox = ui->tabSet->findChild<QDoubleSpinBox*>(e.attribute("name"));
            if (dspbox != nullptr) {
                if (!dspbox->isHidden()) {
                    double value = e.attribute("value").toDouble();
                    if (value != dspbox->value()) setValue(dspbox->objectName().toStdString(), value);
                }
            }
        }
        node = node.nextSibling();  //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    qDebug() << "load ok!";
}
//屏蔽本控件传递事件到父控件
bool Widget::eventFilter(QObject* obj, QEvent* event) {
    Q_UNUSED(obj);
    Q_UNUSED(event);
    if (obj == ui->tableBMU) {
        qDebug() << obj << event;
    }

    return true;  // QWidget::eventFilter(obj, event);
}
void Widget::on_cbProtocol_currentIndexChanged(const QString& arg1) {
    qDebug() << arg1;
    settings->setValue("global/protocol", arg1);
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_SET_PRO;
    MsgCmd.data.setNum(ui->cbProtocol->currentData().toUInt());
    pmq->sendMsg(0, MsgCmd);
    MsgCmd.data.clear();
}

void Widget::initUpdateMenu() {
    update_menu = new QMenu;
    update_menu->addAction("下载升级BMS", this, &Widget::onUpdateBtnMenu);
    update_menu->addAction("下载升级BMU", this, &Widget::onUpdateBtnMenu);
    update_menu->addAction("下载升级BMS Boot", this, &Widget::onUpdateBtnMenu);
    update_menu->addAction("下载升级BMU Boot", this, &Widget::onUpdateBtnMenu);
    update_menu->addAction("下载升级绝缘板", this, &Widget::onUpdateBtnMenu);
    //    update_menu->addAction("升级BMU", this, &Widget::onUpdateBtnMenu);
    ui->btnVer->setMenu(update_menu);
}

void Widget::onUpdateBtnMenu() {
    QAction* b = (QAction*)sender();
    TMsgData MsgCmd;
    if (b->text() == "下载升级BMS") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateCMU};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->text() == "下载升级BMU") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateBMU};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->text() == "下载升级BMS Boot") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateBTC};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->text() == "下载升级BMU Boot") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateBTB};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->text() == "升级BMU") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdBmuNDL};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->text() == "下载升级绝缘板") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdRins};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else {
        return;
    }
    pmq->sendMsg(0, MsgCmd);
    MsgCmd.data.clear();
    return;
}

void Widget::on_checkBox_stateChanged(int arg1) {
    qDebug() << QString("%1").arg(arg1);
    TMsgData MsgCmd;
    QCheckBox* cbox = (QCheckBox*)this->sender();
    if (cbox->isChecked()) {
        MsgCmd.msg_type = CTRL_DUMP;
        MsgCmd.data.clear();
    } else {
        MsgCmd.msg_type = CTRL_DUMP;
        MsgCmd.data.append("0");
    }
    pmq->sendMsg(0, MsgCmd);
    MsgCmd.data.clear();
}
void Widget::btnClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->text();
    if (name == "连接" || name == "重连") {
        uint16_t port = ui->spinBoxPort->value();
        TMsgData MsgCmd;
        MsgCmd.msg_type = CONFIG_IP;
        MsgCmd.data.append(ui->connectIP->text());
        pmq->sendMsg(0, MsgCmd);
        MsgCmd.msg_type = CONFIG_PORT;
        MsgCmd.data.clear();
        MsgCmd.data.append((char*)&port, sizeof(port));
        pmq->sendMsg(0, MsgCmd);
        MsgCmd.msg_type = CONFIG_INIT;
        MsgCmd.data.clear();
        pmq->sendMsg(0, MsgCmd);
    }
}
void Widget::IpChange() {
    QLineEdit* pEdit = (QLineEdit*)sender();
    if (!pEdit->isModified()) return;
    pEdit->setModified(false);
    QString ip = pEdit->text();
    if (!myHelper::IsIP(ip)) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        pEdit->undo();
        return;
    }
    TMsgData MsgCmd;
    MsgCmd.msg_type = CONFIG_IP;
    MsgCmd.data.append(ip);
    pmq->sendMsg(0, MsgCmd);
    settings->setValue("global/target_ip", ip);
}

bool Widget::load_config() {
    settings = new QSettings("config.ini", QSettings::IniFormat);
    QString target_ip = settings->value("global/target_ip", "192.168.1.120").toString();
    int target_port = settings->value("global/target_port", 502).toUInt();
    ui->connectIP->setText(target_ip);
    ui->spinBoxPort->setValue(target_port);
    ui->DataWidget->setCurrentIndex(0);
    return true;
}

void Widget::on_spinBoxPort_valueChanged(int port) { settings->setValue("global/target_port", port); }
