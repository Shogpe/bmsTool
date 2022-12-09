#include "bmsview.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include "Toast.h"
#include "myhelper.h"
#include "socImporter.h"
#include "ui_bmsview.h"
BMSView::BMSView(QWidget* parent) : QWidget(parent), ui(new Ui::BMSView) {
    ui->setupUi(this);
    this->timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &BMSView::timerUpDate);
    mycmu = nullptr;
    bmu_comm = 0;
    pmq = MessageQueue::getInstance();
    pmq->registMsgQueue(99);
    uiInit();
    load_config();
    config = {0, 0, 0, 0, 0};
    //
    QString protocol = QSettings("config.ini", QSettings::IniFormat).value("global/protocol", "CMU1.0").toString();

    ui->cbProtocol->blockSignals(true);
    ui->cbProtocol->clear();
    ui->cbProtocol->addItem("CMU1.0", 0);
    ui->cbProtocol->addItem("CMU2.0", 1);
    ui->cbProtocol->addItem("CMU3.0", 2);
    ui->cbProtocol->addItem("CMU3.1", 6);

    ui->cbProtocol->addItem("CMU4.0", 3);
    //    ui->cbProtocol->addItem("CMU4.1", 4);
    ui->cbProtocol->addItem("CMU4.8", 5);
    for (int i = 0; i < ui->cbProtocol->count(); i++) {
        if (protocol == ui->cbProtocol->itemText(i)) {
            ui->cbProtocol->setCurrentIndex(i);
        }
    }
    ui->cbProtocol->blockSignals(false);

    this->mycmu = new mb_cmu((BMS_PROTOCOL)ui->cbProtocol->currentData().toUInt());
    connect(
        this->mycmu, static_cast<void (mb_cmu::*)(const QString&)>(&mb_cmu::signal_message), this,
        [this](const QString& msg) { Toast::showTip(msg, nullptr); }, Qt::UniqueConnection);
    qRegisterMetaType<QHash<QString, qreal>>("QHash<QString,qreal>");
    connect(this->mycmu, &mb_cmu::bmsDataReady, this, &BMSView::flushData);
    connect(this->mycmu, &mb_cmu::bmuDataReady, this, &BMSView::flushBmu);
    connect(this->mycmu, &mb_cmu::bmsSOEReady, this, &BMSView::flushSoe);
    mycmu->start();
    timer->start(500);
    Toast::showTip(tr("初始化完成"), nullptr);
    qDebug() << ui->DataWidget->sizeHint();
}
bool BMSView::exportExecl(QTableWidget* tableWidget, QString dirFile) {
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
BMSView::~BMSView() {
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

void BMSView::uiChange(QHash<QString, qreal> mapData) {
    qDebug() << "uiChange" << mapData.size();
    ui->tableBMU->setRowCount(config.bmu_num);
    /* 设置 tableWidget */
    //  tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    //  tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    // ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    QStringList hdr_list;
    hdr_list.append(tr("版本号"));
    for (int i = 0; i < config.vol_num; i++) {
        hdr_list.append(("Vol" + QString::number(i + 1)));
    }
    for (int i = 0; i < config.T_num; i++) {
        hdr_list.append(("Tpack" + QString::number(i + 1)));
    }
    for (int i = 0; i < config.Tp_num; i++) {
        hdr_list.append(("Tp" + QString::number(i + 1)));
    }
    hdr_list.append(tr("运行状态"));
    hdr_list.append(tr("故障状态"));
    if (is_main_line(this->mycmu->GetProtocalVer())) {
        if (this->mycmu->GetProtocalVer() > CMUV2) {
            hdr_list.append(tr("CAN错误数"));
        }
        ui->BalnceStart->blockSignals(true);
        ui->BalnceStart->setObjectName("BalnceStart");
        ui->BalnceStart->setPrefix(tr("均衡启动阈值") + " ");
        ui->BalnceStart->setSuffix(" V");
        ui->BalnceStart->setMaximum(6);
        ui->BalnceStart->setDecimals(4);
        ui->BalnceStart->setToolTip(tr("均衡启动阈值"));
        ui->BalnceStart->blockSignals(false);
        ui->BalnceStart->setContextMenuPolicy(Qt::NoContextMenu);
    } else if (is_gender_balanced(this->mycmu->GetProtocalVer())) {
        hdr_list.append(tr("风机"));
        hdr_list.append(tr("母线电压(V)"));
        hdr_list.append(tr("均衡电流(A)"));
        hdr_list.append(tr("均衡故障"));
        hdr_list.append(tr("通道状态"));
        hdr_list.append(tr("均衡模式"));
        hdr_list.append(tr("CAN错误数"));
        // 特殊处理
        ui->BalnceStart->blockSignals(true);
        ui->BalnceStart->setObjectName("BalanceConfig");
        ui->BalnceStart->setPrefix(tr("均衡配置") + " ");
        ui->BalnceStart->setSuffix("");
        ui->BalnceStart->setMaximum(100000);
        ui->BalnceStart->setDecimals(0);
        ui->BalnceStart->setToolTip(tr("均衡配置"));
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
        if (mapData.contains(dspbox->objectName())) {
            dspbox->show();
        }
    }
    // 扩展表格
    if (is_gender_balanced(this->mycmu->GetProtocalVer())) {
        ui->DataWidget->setTabEnabled(ui->DataWidget->indexOf(ui->tabBalance), true);
        QStringList hdr_list2;
        for (int i = 0; i < config.vol_num; i++) {
            hdr_list2.append(tr("充电Ah") + QString::number(i + 1));
            hdr_list2.append(tr("放电Ah") + QString::number(i + 1));
        }
        ui->tableExtView->setRowCount(config.bmu_num);
        ui->tableExtView->setColumnCount(hdr_list2.size());
        ui->tableExtView->setHorizontalHeaderLabels(hdr_list2);
        ui->tableExtView->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
        ui->tableExtView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
    } else {
        ui->DataWidget->setTabEnabled(ui->DataWidget->indexOf(ui->tabBalance), false);
    }

    //
}
int BMSView::setValue(QString name, double dval) {
    NodeReg node = mycmu->GetNodeAddr(name);
    if (node.data_type > 0) {
        uint16_t val[2] = {0};
        val[0] = node.reg_addr;
        //+0.5保障精度
        val[1] = static_cast<uint16_t>(std::round(dval / node.factor));
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_AO_ADDR;
        MsgCmd.data.append(reinterpret_cast<char*>(&val), 2 * sizeof(uint16_t));
        pmq->sendMsg(0, MsgCmd);

    } else {
        qDebug() << "can't find " << name;
    }
    return 0;
}
void BMSView::valueChange() {
    QDoubleSpinBox* b = (QDoubleSpinBox*)sender();
    double dval = b->value();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("要修改\"%1\"为 %2 ?")).arg(b->toolTip()).arg(dval)) !=
        QDialog::Accepted) {
        return;
    }
    b->clearFocus();
    qDebug() << b->objectName() << ":" << dval;
    setValue(b->objectName(), dval);
}

void BMSView::timerUpDate() {
    QTime t;
    t.restart();  //将此时间设置为当前时间
    //
    if (mycmu == nullptr) return;
    if (this->mycmu->drv_status) {
        ui->labelStatus->setStyleSheet("color:green");
        ui->labelStatus->setText(tr("已连接"));
        if (this->mycmu->drv_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("软件过期，请更新！"));
        uint32_t val = this->mycmu->cmu_ver;
        ui->btnVer->setText(QString(tr("版本号:%1")).arg(myHelper::IntegerToHexString(val)));
        ui->tbtnConnect->setText(tr("重连"));
        ui->tbtnConnect->setObjectName("reconnect");
    } else {
        ui->labelStatus->setStyleSheet("color:red;text-decoration:underline;font:bold;");
        ui->labelStatus->setText(tr("未连接"));
        ui->tbtnConnect->setText(tr("连接"));
        ui->tbtnConnect->setObjectName("connect");
    }
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
    // qDebug() << t.elapsed() << "ms";
}
QString BMSView::GetBitStatus(uint16_t value, QString tips) {
    QStringList statusList;
    QStringList tipList;
    if (tips.contains(',')) {
        tipList = tips.split(",");
    } else {
        tipList = QStringList({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
    }
    for (int i = 0; i < tipList.size(); i++) {
        if ((((value >> i) & 0x01) > 0)) statusList << tipList.at(i);
    }
    return statusList.join("|");
}
void BMSView::flushData(int type, QHash<QString, qreal> mapData) {
    if (type == 1) {
        config.bmu_num = mapData.value("bmu_num", 0);
        config.vol_num = mapData.value("vol_num", 0);
        config.T_num = mapData.value("T_num", 0);
        config.Tp_num = mapData.value("Tp_num", 0);
        config.status_num = mapData.value("status_num", 0);
        return uiChange(mapData);
    }
    // memcpy(&config, &mycmu->config, sizeof(config));
    if (config.bmu_num > ui->tableBMU->rowCount()) return;
    uint32_t comm_status1 = mapData.value("sysComm1", 0);
    uint32_t comm_status2 = mapData.value("sysComm2", 0);
    bmu_comm = ((uint64_t)comm_status2 << 32) | comm_status1;
    QString str2 = QString("%1:%2,%3,%4,%5")
                       .arg(tr("BMU拨码异常ID"))
                       .arg(comm_status2 >> 24 & 0xFF, 8, 2, QChar('0'))
                       .arg(comm_status2 >> 16 & 0xFF, 8, 2, QChar('0'))
                       .arg(comm_status2 >> 8 & 0xFF, 8, 2, QChar('0'))
                       .arg(comm_status2 & 0xFF, 8, 2, QChar('0'));
    ui->CommStatus->setText(str2);
    //刷新定值
    QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        if (mapData.contains(dspbox->objectName())) {
            if (dspbox->hasFocus()) continue;
            dspbox->blockSignals(true);
            dspbox->setValue(mapData.value(dspbox->objectName()));
            dspbox->blockSignals(false);
        } else {
        }
    }
    dspboxs = ui->tabCMU->findChildren<QDoubleSpinBox*>();
    dspboxs << ui->sysTime;
    foreach (QDoubleSpinBox* dspbox, dspboxs) {
        if (mapData.contains(dspbox->objectName())) {
            dspbox->show();
            dspbox->setValue(mapData.value(dspbox->objectName()));
        } else {
            dspbox->hide();
        }
    }
    if (mapData.contains("sysStatus1")) {
        uint16_t value = mapData.value("sysStatus1");
        ui->G_SysStatus->setTitle(QString("%1(%2)").arg(tr("系统状态")).arg(value));
        QList<QLabel*> SysStatus;
        SysStatus << ui->bSysErr << ui->bSysAlm << ui->bSysFull << ui->bSysEmpty << ui->bSysInit << ui->bSysCommErr
                  << ui->bSysBalance << ui->bSysCharge << ui->bSysDischarge << ui->bSysStop << ui->bSys10 << ui->bSys11
                  << ui->bSys12 << ui->bSys13 << ui->bSys14 << ui->bSys15;
        QStringList textList;
        textList << tr("总故障") << tr("总告警") << tr("充满") << tr("放空") << tr("未初始化") << tr("BMU通信")
                 << tr("均衡") << tr("充电") << tr("放电") << tr("停机") << tr("升级") << tr("绝缘通信") << tr("自检")
                 << tr("BMU拨码") << tr("BMU故障") << tr("并网");
        foreach (QLabel* Label, SysStatus) {
            QString color = ((value >> SysStatus.indexOf(Label)) & 0x01) > 0
                                ? "color:red;text-decoration:underline;font:bold;"
                                : "color:green;";
            Label->setStyleSheet(QString("%1").arg(color));
            Label->setText(textList.at(SysStatus.indexOf(Label)));
        }
    }
    if (mapData.contains("BootVer")) {
        ui->BootVer->show();
        uint32_t value = mapData.value("BootVer");
        ui->BootVer->setText(QString(tr("Boot版本: %1")).arg(myHelper::IntegerToHexString(value)));
    } else {
        ui->BootVer->hide();
    }
    if (mapData.contains("InsVer")) {
        ui->InsVer->show();
        uint32_t value = mapData.value("InsVer");
        ui->InsVer->setText(QString(tr("绝缘版本: %1")).arg(myHelper::IntegerToHexString(value)));
    } else {
        ui->InsVer->hide();
    }
    if (mapData.contains("sysStatus2")) {
        uint16_t value = mapData.value("sysStatus2");
        ui->G_SysStatus_2->setTitle(QString("%1(%2)").arg(tr("系统状态") + "2").arg(value));
        QList<QLabel*> SysStatus;
        SysStatus << ui->bSysErr_2 << ui->bSysAlm_2 << ui->bSysFull_2 << ui->bSysEmpty_2 << ui->bSysInit_2
                  << ui->bSysCommErr_2 << ui->bSysBalance_2 << ui->bSysCharge_2 << ui->bSysDischarge_2 << ui->bSysStop_2
                  << ui->bSys10_2 << ui->bSys11_2 << ui->bSys12_2 << ui->bSys13_2 << ui->bSys14_2 << ui->bSys15_2;
        QStringList textList;
        textList << tr("IO解锁") << tr("绝缘检测") << tr("") << tr("") << tr("") << tr("") << tr("") << tr("") << tr("")
                 << tr("") << tr("") << tr("") << tr("") << tr("") << tr("") << tr("");
        foreach (QLabel* Label, SysStatus) {
            QString color = ((value >> SysStatus.indexOf(Label)) & 0x01) > 0
                                ? "color:red;text-decoration:underline;font:bold;"
                                : "color:green;";
            Label->setStyleSheet(QString("%1").arg(color));
            Label->setText(textList.at(SysStatus.indexOf(Label)));
            //                if (SysStatus.indexOf(Label) > 2) {
            //                    Label->setHidden(true);
            //                }
        }
    } else {
        ui->G_SysStatus_2->setHidden(true);
    }
    if (mapData.contains("sysErrStatus")) {
        uint16_t value = mapData.value("sysErrStatus");
        ui->G_ErrStatus->setTitle(QString("%1(%2)").arg(tr("保护状态") + "1").arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bErr0 << ui->bErr1 << ui->bErr2 << ui->bErr3 << ui->bErr4 << ui->bErr5 << ui->bErr6
                   << ui->bErr7 << ui->bErr8 << ui->bErr9 << ui->bErr10 << ui->bErr11 << ui->bErr12 << ui->bErr13
                   << ui->bErr14 << ui->bErr15;
        foreach (QLabel* Label, StatusList) {
            QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                ? "color:red;text-decoration:underline;font:bold;"
                                : "color:green;";
            Label->setStyleSheet(QString("%1").arg(color));
        }
    }
    if (mapData.contains("sysErrStatus2")) {
        uint16_t value = mapData.value("sysErrStatus2");
        ui->G_ErrStatus_2->setTitle(QString("%1(%2)").arg(tr("保护状态") + "2").arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bErr0_2 << ui->bErr1_2 << ui->bErr2_2 << ui->bErr3_2 << ui->bErr4_2 << ui->bErr5_2
                   << ui->bErr6_2 << ui->bErr7_2 << ui->bErr8_2 << ui->bErr9_2 << ui->bErr10_2 << ui->bErr11_2
                   << ui->bErr12_2 << ui->bErr13_2 << ui->bErr14_2 << ui->bErr15_2;
        foreach (QLabel* Label, StatusList) {
            QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                ? "color:red;text-decoration:underline;font:bold;"
                                : "color:green;";
            Label->setStyleSheet(QString("%1").arg(color));
        }
    }
    if (mapData.contains("sysAlmStatus")) {
        uint16_t value = mapData.value("sysAlmStatus");
        ui->G_AlmStatus->setTitle(QString("%1(%2)").arg(tr("告警状态") + "1").arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bAlm0 << ui->bAlm1 << ui->bAlm2 << ui->bAlm3 << ui->bAlm4 << ui->bAlm5 << ui->bAlm6
                   << ui->bAlm7 << ui->bAlm8 << ui->bAlm9 << ui->bAlm10 << ui->bAlm11 << ui->bAlm12 << ui->bAlm13
                   << ui->bAlm14 << ui->bAlm15;
        foreach (QLabel* Label, StatusList) {
            QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                ? "color:gold;text-decoration:underline;font:bold;"
                                : "color:green;";
            Label->setStyleSheet(QString("%1").arg(color));
        }
    }
    if (mapData.contains("sysAlmStatus2")) {
        uint16_t value = mapData.value("sysAlmStatus2");
        ui->G_AlmStatus_2->setTitle(QString("%1(%2)").arg(tr("告警状态") + "2").arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bAlm0_2 << ui->bAlm1_2 << ui->bAlm2_2 << ui->bAlm3_2 << ui->bAlm4_2 << ui->bAlm5_2
                   << ui->bAlm6_2 << ui->bAlm7_2 << ui->bAlm8_2 << ui->bAlm9_2 << ui->bAlm10_2 << ui->bAlm11_2
                   << ui->bAlm12_2 << ui->bAlm13_2 << ui->bAlm14_2 << ui->bAlm15_2;
        QStringList textList = {tr("BMU拨码异常"),    tr("电压线束断线"),   tr("温度线束断线"),
                                tr("簇极柱温度断线"), tr("电压传感器断线"), tr("电流传感器断线"),
                                tr("断路器拒动"),     tr("接触器拒动"),     tr("备用8")};
        textList << tr("备用9") << tr("备用10") << tr("备用11") << tr("备用12") << tr("备用13") << tr("备用14")
                 << tr("备用15") << tr("备用16");
        foreach (QLabel* Label, StatusList) {
            int index = StatusList.indexOf(Label);
            if (index < textList.size()) {
                QString color =
                    ((value >> index) & 0x01) > 0 ? "color:gold;text-decoration:underline;font:bold;" : "color:green;";
                Label->setStyleSheet(QString("%1").arg(color));
                Label->setText(textList.at(index));
                Label->setHidden(false);
            } else {
                Label->setHidden(true);
            }
        }
    }
    if (mapData.contains("sysDIStatus")) {
        uint16_t value = mapData.value("sysDIStatus");
        ui->G_DIStatus->setTitle(QString("%1(%2)").arg(tr("DI状态")).arg(value));
        QList<QLabel*> StatusList;
        StatusList << ui->bDI0 << ui->bDI1 << ui->bDI2 << ui->bDI3 << ui->bDI4 << ui->bDI5 << ui->bDI6 << ui->bDI7
                   << ui->bDI8 << ui->bDI9 << ui->bDI10 << ui->bDI11 << ui->bDI12 << ui->bDI13 << ui->bDI14
                   << ui->bDI15;
        QStringList textList;
        textList << tr("QF状态") << tr("KM+状态") << tr("KM-状态") << tr("KMR状态") << tr("故障输入") << tr("主从状态")
                 << tr("预留DIN1(水浸)") << tr("交流有压") << tr("急停保护") << tr("QF继电器状态")
                 << tr("故障输出继电器状态") << tr("BMU风扇继电器状态") << tr("高压箱风扇继电器状态")
                 << tr("充满继电器状态") << tr("放空继电器状态") << tr("备用16");
        foreach (QLabel* Label, StatusList) {
            QString color = ((value >> StatusList.indexOf(Label)) & 0x01) > 0
                                ? "color:red;text-decoration:underline;font:bold;"
                                : "color:green;";
            Label->setStyleSheet(QString("%1").arg(color));
            Label->setText(textList.at(StatusList.indexOf(Label)));
        }
    }
    if (mapData.contains("sysDOStatus")) {
        uint16_t value = mapData.value("sysDOStatus");
        ui->G_DOStatus->setTitle(QString("%1(%2)").arg(tr("DO状态")).arg(value));
        QList<QCheckBox*> RadioList;
        RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
                  << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
        QStringList textList;
        textList << tr("QF输出") << tr("KM+输出") << tr("KM-输出") << tr("KMR输出") << tr("故障输出") << tr("充电指示")
                 << tr("放电指示") << tr("系统运行") << tr("BMU供电") << tr("告警输出") << tr("风扇电源输出")
                 << tr("备用12") << tr("充满输出") << tr("放空输出") << tr("备用15") << tr("备用16");
        foreach (QCheckBox* rb, RadioList) {
            bool bit = ((value >> RadioList.indexOf(rb)) & 0x01) > 0;
            QString color = bit ? "color:red;text-decoration:underline;font:bold;" : "color:green;";
            rb->setStyleSheet(QString("%1").arg(color));
            rb->blockSignals(true);
            rb->setChecked(bit);
            rb->blockSignals(false);
            rb->setText(textList.at(RadioList.indexOf(rb)));
        }
    }
    if (mapData.contains("FuncMask")) {
        uint16_t value = mapData.value("FuncMask");
        ui->G_FuncMask->setTitle(QString("%1(%2)").arg(tr("使能位")).arg(value));
        QList<QCheckBox*> CheckBoxList;
        CheckBoxList << ui->bFunc0 << ui->bFunc1 << ui->bFunc2 << ui->bFunc3 << ui->bFunc4 << ui->bFunc5 << ui->bFunc6
                     << ui->bFunc7 << ui->bFunc8 << ui->bFunc9 << ui->bFunc10 << ui->bFunc11 << ui->bFunc12
                     << ui->bFunc13 << ui->bFunc14 << ui->bFunc15;
        QStringList textList;
        textList << tr("使能双CAN") << tr("使能电流传感器") << tr("使能电压传感器") << tr("使能漏电流传感器")
                 << tr("使能绝缘检测") << tr("使能写保护") << tr("使能故障录波") << tr("禁用定值限制") << tr("使能环控")
                 << tr("禁用远控接触器") << tr("网络输出使能") << tr("调试输出使能") << tr("单簇/多簇")
                 << tr("并列/解列") << tr("禁用预充") << tr("禁用安防");
        if (is_gender_balanced(this->mycmu->GetProtocalVer())) {
            textList.replace(0, tr("使能预留传感器"));
            textList.replace(1, tr("使能绝缘板采样电压"));
        }
        foreach (QCheckBox* cb, CheckBoxList) {
            cb->blockSignals(true);
            cb->setChecked(((value >> CheckBoxList.indexOf(cb)) & 0x01) > 0);
            cb->blockSignals(false);
            cb->setText(textList.at(CheckBoxList.indexOf(cb)));
        }
    }
    if (mapData.contains("BalnceMask") && mapData.contains("BalanceConfig")) {
        ui->balanceStr->show();
        uint16_t mode = mapData.value("BalnceMask");
        uint16_t value = mapData.value("BalanceConfig");
        QString str = QString(tr("%1对,%2A,%3秒")).arg(value >> 12).arg((value >> 8) & 0xF).arg(value & 0xFF);
        switch (mode) {
            case 0x00:
                str = tr("禁止均衡") + QString(":%1").arg(str);
                break;
            case 0x55:
                str = tr("强制均衡") + QString(":%1").arg(str);
                break;
            case 0xAA:
                str = tr("自动均衡") + QString(":%1").arg(str);
                break;
            case 0x88:
                str = tr("手动均衡") + QString(":%1").arg(str);
                break;
            default:
                str = tr("未定义") + QString(":%1").arg(str);
                break;
        }

        ui->balanceStr->setText(str);

    } else if (mapData.contains("BalnceMask")) {
        ui->balanceStr->show();
        uint16_t mode = mapData.value("BalnceMask");
        QString str;
        switch (mode) {
            case 0x00:
                str = tr("禁止均衡");
                break;
            case 0x55:
                str = tr("强制均衡");
                break;
            case 0xAA:
                str = tr("自动均衡");
                break;
            case 0x88:
                str = tr("手动均衡");
                break;
            default:
                str = tr("未定义");
                break;
        }

        ui->balanceStr->setText(str);
    } else {
        ui->balanceStr->hide();
    }

    if (!ui->lineEditServIP->hasFocus()) {
        if (mapData.contains("ServIP"))
            ui->lineEditServIP->setText(myHelper::IPV4IntegerToString(mapData.value("ServIP")));
    }
    if (!ui->lineEditIP->hasFocus()) {
        if (mapData.contains("LocalIP"))
            ui->lineEditIP->setText(myHelper::IPV4IntegerToString(mapData.value("LocalIP")));
    }
    uint16_t id = mapData.value("UmaxID");
    ui->UmaxID->setText(QString("%1(%2)").arg(tr("最大单体电压"), myHelper::IDToString(id, config.vol_num)));
    id = mapData.value("UminID");
    ui->UminID->setText(QString("%1(%2)").arg(tr("最小单体电压"), myHelper::IDToString(id, config.vol_num)));
    id = mapData.value("TmaxID");
    ui->TmaxID->setText(QString("%1(%2)").arg(tr("最高单体温度"), myHelper::IDToString(id, config.T_num)));
    id = mapData.value("TminID");
    ui->TminID->setText(QString("%1(%2)").arg(tr("最低单体温度"), myHelper::IDToString(id, config.T_num)));
    id = mapData.value("UmMaxID");
    ui->UmMaxID->setText(QString("%1(%2)").arg(tr("最大模组电压"), myHelper::IDToString(id, config.vol_num)));
    id = mapData.value("UdMaxID");
    ui->UdMaxID->setText(QString("%1(%2)").arg(tr("最大单体压差"), myHelper::IDToString(id, config.vol_num)));
    id = mapData.value("TpMaxID");
    ui->TpMaxID->setText(QString("%1(%2)").arg(tr("最大极柱温度"), myHelper::IDToString(id, config.Tp_num)));
    id = mapData.value("TrMaxID");
    ui->TrMaxID->setText(QString("%1(%2)").arg(tr("最大单体温升"), myHelper::IDToString(id, config.T_num)));
}
QString getBmuInfo2(uint16_t status) {
    QStringList statusList;
    if (GET_BIT(status, 0)) statusList << "拨码异常";
    if (GET_BIT(status, 1)) statusList << "拨码锁定";
    statusList << (GET_BIT(status, 2) ? "干结点开路" : "干结点闭合");
    statusList << (GET_BIT(status, 3) ? "风机开" : "风机关");
    if (!GET_BIT(status, 4)) statusList << "辅源异常";
    if (GET_BIT(status, 5)) statusList << "备用5";
    if (GET_BIT(status, 6)) statusList << "备用6";
    if (GET_BIT(status, 7)) statusList << "备用7";
    if (GET_BIT(status, 8)) statusList << "1.25V错误";
    if (GET_BIT(status, 9)) statusList << "均衡母线错误";
    if (GET_BIT(status, 10)) statusList << "均衡电流异常";
    if (GET_BIT(status, 11)) statusList << "24V母线异常";
    if (GET_BIT(status, 12)) statusList << "单体电压异常";
    if (GET_BIT(status, 13)) statusList << "均衡参数错误";
    if (GET_BIT(status, 14)) statusList << "Mos异常";
    if (GET_BIT(status, 15)) statusList << "副边电压异常";
    // if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
    return statusList.join("|");
}
void BMSView::flushSoe(const ST_SOE& soe) {
    if (soe.list_soe.count()) {
        ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        m_model.setData(soe.list_soe, db_manager::SOE_BMS2);
        ui->labelSOE->setText(QString("New:%1,Total:%2").arg(soe.new_soe_count).arg(soe.soe_count));
        ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}
void BMSView::flushBmu() {
    if (!mycmu) return;
    if (config.bmu_num > ui->tableBMU->rowCount()) return;

    //一定要固定宽度，否则刷新很慢
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int cloumn_offset = 0;
    QTableWidgetItem* item;
    for (int i = 0; i < config.bmu_num; i++) {
        cloumn_offset = 0;
        //版本号
        item = new QTableWidgetItem();
        item->setText(myHelper::IntegerToHexString(mycmu->bmu_data[i].Version));
        QFont font = item->font();
        if (this->bmu_comm >> i & 0x01) {
            item->setTextColor(QColor(Qt::darkGreen));
            font.setStrikeOut(false);
            font.setBold(false);
        } else {
            item->setTextColor(QColor(Qt::red));
            font.setStrikeOut(true);
            font.setBold(true);
        }
        item->setFont(font);
        item->setToolTip(tr("Strikethrough indicates disconnection"));
        //        item->setToolTip("删除线表示断线");
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableBMU->setItem(i, cloumn_offset++, item);
        //单体电压
        for (int j = 0; j < config.vol_num; j++) {
            item = new QTableWidgetItem();
            double val = this->mycmu->bmu_data[i].Ucell[j] / 10000.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            QFont font = item->font();
            if (GET_BIT(mycmu->bmu_data[i].Ubreak, j)) {
                font.setStrikeOut(true);
            } else {
                font.setStrikeOut(false);
            }
            // pack最大标红
            if (mycmu->bmu_data[i].MaxUcellId == j) {
                item->setTextColor(QColor(Qt::red));
                // 簇最大标粗
                if (mycmu->bms_data.MaxUcellId == i) {
                    font.setBold(true);
                }
            }
            if (mycmu->bmu_data[i].MinUcellId == j) {
                item->setTextColor(QColor(Qt::darkGreen));
                if (mycmu->bms_data.MinUcellId == i) {
                    font.setItalic(true);
                }
            }
            //
            item->setFont(font);
            item->setToolTip(tr("Strikethrough indicates disconnection"));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
        }

        cloumn_offset += config.vol_num;

        for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = this->mycmu->bmu_data[i].Tcell[j] / 10.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            QFont font = item->font();
            if (GET_BIT(mycmu->bmu_data[i].Tbreak, j)) {
                font.setStrikeOut(true);
            } else {
                font.setStrikeOut(false);
            }

            // pack最大标红
            if (mycmu->bmu_data[i].MaxTcellId == j) {
                item->setTextColor(QColor(Qt::red));
                // 簇最大标粗
                if (mycmu->bms_data.MaxTcellId == i) {
                    font.setBold(true);
                }
            }
            if (mycmu->bmu_data[i].MinTcellId == j) {
                item->setTextColor(QColor(Qt::darkGreen));
                // 簇最小标斜体
                if (mycmu->bms_data.MinTcellId == i) {
                    font.setItalic(true);
                }
            }
            item->setFont(font);
            item->setToolTip(tr("Strikethrough indicates disconnection"));
            ui->tableBMU->setItem(i, j + cloumn_offset, item);
        }

        cloumn_offset += (config.T_num + config.Tp_num);

        // 电压断线，温度断线，运行状态，故障状态
        //        item = new QTableWidgetItem();
        //        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].Ubreak, 4, 16, QLatin1Char('0')));
        //        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        //        item->setToolTip(GetBitStatus(mycmu->bmu_data[i].Ubreak));
        //        //        if (0 == i) qDebug() << GetBitStatus(mycmu->bmu_data[i].Ubreak);
        //        ui->tableBMU->setItem(i, cloumn_offset++, item);

        //        item = new QTableWidgetItem();
        //        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].Tbreak, 4, 16, QLatin1Char('0')));
        //        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        //        item->setToolTip(GetBitStatus(mycmu->bmu_data[i].Tbreak));
        //        ui->tableBMU->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].RunStat, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setToolTip(getBmuInfo2(mycmu->bmu_data[i].RunStat));
        ui->tableBMU->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].ErrStat, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setToolTip(GetBitStatus(mycmu->bmu_data[i].ErrStat));
        ui->tableBMU->setItem(i, cloumn_offset++, item);

        if (is_gender_balanced(this->mycmu->GetProtocalVer())) {
            item = new QTableWidgetItem();
            QString fanStatus = GET_BIT(mycmu->bmu_data[i].RunStat, 3) ? tr("ON") : tr("OFF");
            item->setText(fanStatus);
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);

            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(mycmu->bmu_data[i].BalU24 / 1000.0));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(mycmu->bmu_data[i].BalIdc / 1000.0));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(GetBitStatus(mycmu->bmu_data[i].BalErr));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
            item = new QTableWidgetItem();
            item->setText(GetBitStatus(mycmu->bmu_data[i].BalStat));
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
        } else if (mycmu->GetProtocalVer() > CMUV2) {
            // CAN通信错误计数
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(this->mycmu->bmu_data[i].CanErr));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, cloumn_offset++, item);
        }
    }
    //数据刷新完毕后自适应列宽
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    if (ui->tabBalance->isEnabled()) {
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
static map<QString, mb_cmd> btnMap = {
    {"btnBMULock", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_BMU_LOCK}},
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
    {"btnRebootCMU", {CTRL_CMD_REBOOT, ADDR_REBOOT, MB_REBOOT}},
    {"btnRebootBMU", {CTRL_CMD_REBOOT, ADDR_REBOOT, MB_REBOOT_BMU}},
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
    //                                      {"btnFanON", {CTRL_AO_ADDR, ADDR_CTRL_FAN, MB_CTRL_ON}},
    //                                      {"btnFanOFF", {CTRL_AO_ADDR, ADDR_CTRL_FAN, MB_CTRL_OFF}},
    {"btnAcON", {CTRL_AO_ADDR, ADDR_CTRL_AC, MB_CTRL_ON}},
    {"btnAcOFF", {CTRL_AO_ADDR, ADDR_CTRL_AC, MB_CTRL_OFF}},
    {"btnResON", {CTRL_AO_ADDR, ADDR_CTRL_RES, MB_CTRL_ON}},
    {"btnResOFF", {CTRL_AO_ADDR, ADDR_CTRL_RES, MB_CTRL_OFF}},
    {"btnTimeAdj", {CERT_CMD_TIME_ADJ, 0, 0}},
    {"btnResetDef", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_FACTORY}}};

void BMSView::sendCommand() {
    TMsgData MsgCmd;
    uint16_t val[3];
    QString text = "";
    QString name = "";
    QPushButton* btn = qobject_cast<QPushButton*>(QObject::sender());
    if (btn) {
        text = btn->text();
        name = btn->objectName();
    } else {
        QAction* act = qobject_cast<QAction*>(QObject::sender());
        if (act) {
            text = act->text();
            name = act->objectName();
        } else {
            myHelper::ShowMessageBoxError("no such command!");
            return;
        }
    }
    map<QString, mb_cmd>::iterator iter1;
    iter1 = btnMap.find(name);
    if (iter1 != btnMap.end()) {
        if (myHelper::ShowMessageBoxQuesion(QString("%1 %2 ？").arg(tr("是否执行"), text)) == QDialog::Accepted) {
            mb_cmd cmd = iter1->second;
            MsgCmd.msg_type = cmd.type;
            MsgCmd.data.append(reinterpret_cast<char*>(&cmd.addr), sizeof(uint16_t));
            MsgCmd.data.append(reinterpret_cast<char*>(&cmd.value), sizeof(uint16_t));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        }
    } else if (name == "btnRUAdj") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = ADDR_RINS_ADJ;
        val[1] = MB_RU_ADJ;
        bool lbok;
        QString value = myHelper::showInputBox(tr("绝缘电压校准值"), lbok);
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
        QString value = myHelper::showInputBox(tr("正绝缘电阻校准值"), lbok);
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
        QString value = myHelper::showInputBox(tr("负绝缘电阻校准值"), lbok);
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
        mode = ui->BalnceMask->value();
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
    } else if (name == "btnBalClrErr") {
        if (myHelper::ShowMessageBoxQuesion(tr("是否清除均衡故障？")) == QDialog::Accepted) {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t value[2] = {65289, 0xAA55};  // 0xFF09
            MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        }
    } else if (name == "btnSetSOC") {
        MsgCmd.msg_type = CTRL_AO_ADDR;
        val[0] = 0xFFF6;
        bool lbok;
        QString value = myHelper::showInputBox(tr("SOC标定值"), lbok);
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
        QString value = myHelper::showInputBox(tr("SOH标定值"), lbok);
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
void BMSView::stateChanged() {
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
    NodeReg node = mycmu->GetNodeAddr("FuncMask");
    if (node.reg_type > 0) {
        value[0] = node.reg_addr;
        MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
        pmq->sendMsg(0, MsgCmd);
    }
}
void BMSView::checkChanged() {
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
    QMessageBox box(QMessageBox::Warning, tr("输出控制"), QString("%1:%2").arg(tr("当前控制出口为"), b->text()));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    box.setButtonText(QMessageBox::Yes, QString(tr("控 合")));
    box.setButtonText(QMessageBox::No, QString(tr("控 分")));
    box.setButtonText(QMessageBox::Cancel, QString(tr("取 消")));
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
void BMSView::btn_contrl() {
    TMsgData MsgCmd;
    QPushButton* b = (QPushButton*)sender();
    QString name = b->objectName();
    if (name == "btnReadSOE") {
        MsgCmd.msg_type = CERT_CMD_READ_SOE;
        MsgCmd.data.clear();
        pmq->sendMsg(0, MsgCmd);
        ui->labelSOE->setText(tr("读取中...请稍侯..."));
    } else if (name == "btnClearSOE") {
        if (myHelper::ShowMessageBoxQuesion("Sure to clear All SOE ?") == QDialog::Accepted) {
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t val[2] = {0xFFF8, 0xBB66};
            MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            pmq->sendMsg(0, MsgCmd);
            MsgCmd.data.clear();
        }
    } else
        qDebug() << name;
}

void BMSView::on_lineEditIP_editingFinished() {
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
    val[0] = 5418;
    ip = bswap_32(ip);
    val[1] = ip & 0xFFFF;
    val[2] = ip >> 16 & 0xFFFF;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO_ADDR;
    MsgCmd.data.append((char*)&val, 3 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}

void BMSView::on_lineEditServIP_editingFinished() {
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
    val[0] = 5420;
    ip = bswap_32(ip);
    val[1] = ip & 0xFFFF;
    val[2] = ip >> 16 & 0xFFFF;
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO_ADDR;
    MsgCmd.data.append((char*)&val, 3 * sizeof(uint16_t));
    pmq->sendMsg(0, MsgCmd);
}
bool BMSView::saveParameters(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        myHelper::ShowMessageBoxError(tr("file not opened."));
        return false;
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
            item1.setAttribute("name_cn", dspbox->toolTip().trimmed());
            item1.setAttribute("value", QString::number(dspbox->value()));
            root_elem.appendChild(item1);
        }
    }
    QTextStream out(&file);
    document.save(out, 4);
    file.close();
    return true;
}
bool BMSView::loadParameters(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        myHelper::ShowMessageBoxError(tr("file not found."));
        return false;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        myHelper::ShowMessageBoxError(tr("not invalid file."));
        return false;
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
                    if (value != dspbox->value()) setValue(dspbox->objectName(), value);
                }
            }
        }
        node = node.nextSibling();  //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    qDebug() << "load ok!";
    return true;
}
void BMSView::on_btnOutput_released() {
    QString filename = QFileDialog::getSaveFileName(this, "Save", "", "*.xml");
    if (!filename.isEmpty()) {
        if (saveParameters(filename)) {
            Toast::showTip("save parameters ok!");
        } else {
            Toast::showTip("save parameters failed!!!!!");
        }
    }
}

void BMSView::on_btnInput_released() {
    QString filename = QFileDialog::getOpenFileName(this, "Open", "", "*.xml");
    if (!filename.isEmpty()) {
        if (loadParameters(filename)) {
            Toast::showTip("load parameters ok!");
        } else {
            Toast::showTip("load parameters failed!!!!!");
        }
    }
}
//屏蔽本控件传递事件到父控件
bool BMSView::eventFilter(QObject* obj, QEvent* event) {
    Q_UNUSED(obj);
    Q_UNUSED(event);
    if (obj == ui->tableBMU) {
        qDebug() << obj << event;
    }

    return true;  // QWidget::eventFilter(obj, event);
}
void BMSView::on_cbProtocol_currentIndexChanged(const QString& arg1) {
    qDebug() << arg1;
    settings->setValue("global/protocol", arg1);
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_SET_PRO;
    MsgCmd.data.setNum(ui->cbProtocol->currentData().toUInt());
    pmq->sendMsg(0, MsgCmd);
    MsgCmd.data.clear();
}

void BMSView::uiInit() {
    update_menu = new QMenu;
    update_menu->addAction(tr("下载升级BMS"), this, &BMSView::onUpdateBtnMenu);
    update_menu->actions().constLast()->setObjectName("upgradeBMS");
    update_menu->addAction(tr("下载升级BMU"), this, &BMSView::onUpdateBtnMenu);
    update_menu->actions().constLast()->setObjectName("upgradeBMU");
    update_menu->addAction(tr("下载升级BMS Boot"), this, &BMSView::onUpdateBtnMenu);
    update_menu->actions().constLast()->setObjectName("upgradeBMSBoot");
    update_menu->addAction(tr("下载升级BMU Boot"), this, &BMSView::onUpdateBtnMenu);
    update_menu->actions().constLast()->setObjectName("upgradeBMUBoot");
    update_menu->addAction(tr("下载升级绝缘板"), this, &BMSView::onUpdateBtnMenu);
    update_menu->actions().constLast()->setObjectName("upgradeINS");
    ui->btnVer->setMenu(update_menu);
    {
        connect(ui->connectIP, &QLineEdit::editingFinished, this, &BMSView::IpChange, Qt::UniqueConnection);
        connect(ui->tbtnConnect, SIGNAL(clicked(bool)), this, SLOT(btnClick()));
        //
        QList<QDoubleSpinBox*> dspboxs = ui->tabSet->findChildren<QDoubleSpinBox*>();
        foreach (QDoubleSpinBox* dspbox, dspboxs) {
            connect(dspbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                    &BMSView::valueChange, Qt::UniqueConnection);
        }
        connect(ui->BalnceMask,
                static_cast<void (QDoubleSpinBox::*)(const QPoint& pos)>(&QDoubleSpinBox::customContextMenuRequested),
                this,
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
                static_cast<void (QDoubleSpinBox::*)(const QPoint& pos)>(&QDoubleSpinBox::customContextMenuRequested),
                this,
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
            connect(btn, &QPushButton::released, this, &BMSView::sendCommand, Qt::UniqueConnection);
        }
        QList<QCheckBox*> chkboxs = ui->G_FuncMask->findChildren<QCheckBox*>();
        foreach (QCheckBox* chkbox, chkboxs) {
            connect(chkbox, &QCheckBox::stateChanged, this, &BMSView::stateChanged, Qt::UniqueConnection);
        }
        QList<QCheckBox*> RadioList;
        RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
                  << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
        foreach (QCheckBox* rb, RadioList) {
            connect(rb, &QCheckBox::stateChanged, this, &BMSView::checkChanged, Qt::UniqueConnection);
        }  //
        connect(ui->btnReadSOE, &QPushButton::released, this, &BMSView::btn_contrl, Qt::UniqueConnection);
        connect(ui->btnClearSOE, &QPushButton::released, this, &BMSView::btn_contrl, Qt::UniqueConnection);

        ui->ViewSOE->verticalHeader()->hide();
        ui->ViewSOE->horizontalHeader()->setStretchLastSection(true);
        ui->ViewSOE->setModel(&m_model);
        ui->ViewSOE->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->ViewSOE,
                static_cast<void (QTableView::*)(const QPoint& pos)>(&QTableView::customContextMenuRequested), this,
                [=](const QPoint& pos) {  // Handle global position
                    QPoint globalPos = ui->ViewSOE->mapToGlobal(pos);

                    // Create menu and insert some actions
                    QMenu myMenu;
                    myMenu.addAction(tr("导出当前SOE"), this, [=]() {
                        QString fileName = QFileDialog::getSaveFileName(
                            this, tr("Save File"),
                            tr("SOE导出") + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
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
                &BMSView::pop_bmuTable_menu);
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
    }
    // 按钮
    QMenu* rebootMenu = new QMenu(this);
    rebootMenu->addAction(tr("重启CMU"), this, &BMSView::sendCommand);
    rebootMenu->actions().constLast()->setObjectName("btnRebootCMU");
    rebootMenu->addAction(tr("重启BMU"), this, &BMSView::sendCommand);
    rebootMenu->actions().constLast()->setObjectName("btnRebootBMU");
    ui->btnReboot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->btnReboot->setMenu(rebootMenu);
}

void BMSView::onUpdateBtnMenu() {
    QAction* b = (QAction*)sender();
    TMsgData MsgCmd;
    if (b->objectName() == "upgradeBMS") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateCMU};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->objectName() == "upgradeBMU") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateBMU};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->objectName() == "upgradeBMSBoot") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateBTC};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->objectName() == "upgradeBMUBoot") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateBTB};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } else if (b->objectName() == "upgradeINS") {
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

void BMSView::on_checkBox_stateChanged(int arg1) {
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
void BMSView::btnClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->objectName();
    if (name == "connect" || name == "reconnect") {
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
void BMSView::IpChange() {
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

bool BMSView::load_config() {
    settings = new QSettings("config.ini", QSettings::IniFormat);
    QString target_ip = settings->value("global/target_ip", "192.168.1.120").toString();
    int target_port = settings->value("global/target_port", 502).toUInt();
    ui->connectIP->setText(target_ip);
    ui->spinBoxPort->setValue(target_port);
    ui->DataWidget->setCurrentIndex(0);
    return true;
}

void BMSView::on_spinBoxPort_valueChanged(int port) { settings->setValue("global/target_port", port); }

void BMSView::on_btnSaveDefault_released() {
    //    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要保存当前参数为默认值吗？"))) != QDialog::Accepted) {
    //        return;
    //    }
    if (saveParameters("default.xml")) {
        Toast::showTip("save parameters ok!");
    } else {
        Toast::showTip("save parameters failed!!!!!");
    }
}

void BMSView::on_btnLoadDefault_released() {
    //    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要加载上次保存的默认参数吗？"))) != QDialog::Accepted)
    //    {
    //        return;
    //    }
    if (loadParameters("default.xml")) {
        Toast::showTip("load parameters ok!");
    } else {
        Toast::showTip("load parameters failed!!!!!");
    }
}
void BMSView::pop_bmuTable_menu(const QPoint& pos) {
    {  // Handle global position
        QTableWidget* table = ui->tableBMU;
        //        QPoint globalPos = table->mapToGlobal(pos);
        QModelIndex index = table->indexAt(pos);
        qDebug() << index.row();
        // Create menu and insert some actions
        QMenu* myMenu = new QMenu(table);
        myMenu->addAction(tr("导出当前数据"), this, [=]() {
            QString fileName = QFileDialog::getSaveFileName(
                this, tr("Save File"), tr("BMU数据") + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
                tr("csv File(*.csv)"));
            if (fileName.isNull()) {
                return;
            }
            exportExecl(table, fileName);
        });
        myMenu->addAction(QString("%1:BMU%2").arg(tr("开启风扇")).arg(index.row() + 1), this, [this, index]() {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t val[2] = {0xFF0B, index.row() | 0xA500};
            MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        });
        myMenu->addAction(QString("%1:BMU%2").arg(tr("关闭风扇")).arg(index.row() + 1), this, [this, index]() {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t val[2] = {0xFF0B, index.row() | 0x5A00};
            MsgCmd.data.append(reinterpret_cast<char*>(&val), 2 * sizeof(val[0]));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        });
        myMenu->addAction(tr("开启全部风扇"), this, [this]() {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t val[2] = {0xFF0B, 0xA5FE};
            MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        });
        myMenu->addAction(tr("关闭全部风扇"), this, [this, index]() {
            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t val[2] = {0xFF0B, 0xA5FF};
            MsgCmd.data.append(reinterpret_cast<char*>(&val), 2 * sizeof(val[0]));
            if (MsgCmd.data.size() > 0) pmq->sendMsg(0, MsgCmd);
        });
        myMenu->move(cursor().pos());
        myMenu->show();
        myMenu->setAttribute(Qt::WA_DeleteOnClose);
        // Show context menu at handling position
        //        myMenu.exec(globalPos);
    }
}
void BMSView::changeEvent(QEvent* event) {
    if (0 != event) {
        switch (event->type()) {
            // this event is send if a translator is loaded
            case QEvent::LanguageChange: {
                ui->retranslateUi(this);
                break;
            }
            default: {
                break;
            }
        }
    }

    QWidget::changeEvent(event);
}
