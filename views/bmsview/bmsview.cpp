#include "bmsview.h"
#include <QDateTime>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include <Rebootbmus.h>
#include "SwitchPowerConfig.h"
#include "Toast.h"
#include "myhelper.h"
#include "socImporter.h"
#include "ui_bmsview.h"
#include <QTcpSocket>
#include <QFile>
#include "sysstatwd.h"
#include <QDialog>
#include <QScrollArea>
#include <QLabel>
#include <QHBoxLayout>
#include "protocolsetframe.h"


/**
proxy style for text wrapping in pushbutton
*/
class QtPushButtonStyleProxy : public QProxyStyle {
public:
    /**
    Default constructor.
    */
    QtPushButtonStyleProxy() : QProxyStyle() {}

    virtual void drawItemText(QPainter* painter, const QRect& rect, int flags, const QPalette& pal, bool enabled,
                              const QString& text, QPalette::ColorRole textRole) const {
        flags |= Qt::TextWordWrap;
        QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
    }

private:
    Q_DISABLE_COPY(QtPushButtonStyleProxy)
};

#define UI_IP_LE    1
#define UI_IP_CB    2
#define UI_IP_SEL   UI_IP_CB


void BMSView::initUserLevelForm()
{
    guestHideInputBoxList.clear();
    if(db_manager::Instance()->userLevel() == db_manager::LEVEL_GUEST)
    {
        ui->btn_debugLog->setVisible(false);
        ui->btnImportSOC->setVisible(false);

        ui->groupBoxDebugReg->setVisible(false);

        ui->gp_sysPara1->setVisible(true);
        ui->gp_sysPara2->setVisible(false);
        ui->gp_alarmValue1->setVisible(false);
        ui->gp_alarmValue2->setVisible(false);
        ui->gp_sysPara3->setVisible(false);
        ui->gp_muskBlock->setVisible(false);
        ui->gp_filePort->setVisible(false);
        ui->wd_fillLimit->setVisible(true);

        ui->G_SysStatus->setVisible(false);
        ui->G_DIStatus->setVisible(false);
        ui->G_DOStatus->setVisible(false);
        ui->G_ErrStatus->setVisible(false);
        ui->G_AlmStatus->setVisible(false);
        ui->G_PreAlmStatus->setVisible(false);

        ui->pb_modBusHelp->setVisible(false);
        //簇数据隐藏
        guestHideInputBoxList.append(ui->TpMax);
        guestHideInputBoxList.append(ui->TpMaxID);
        guestHideInputBoxList.append(ui->TmcopperMax);
        guestHideInputBoxList.append(ui->TmcopperMin);
        guestHideInputBoxList.append(ui->ClusterT5);
        guestHideInputBoxList.append(ui->ClusterT3);
        guestHideInputBoxList.append(ui->ClusterT4);
        guestHideInputBoxList.append(ui->ClusterT1);
        guestHideInputBoxList.append(ui->ClusterT2);
        guestHideInputBoxList.append(ui->Pdc);
        guestHideInputBoxList.append(ui->CapRemain);
        guestHideInputBoxList.append(ui->CapCurCharge);
        guestHideInputBoxList.append(ui->CapCurDischarge);
        guestHideInputBoxList.append(ui->CapCharge);
        guestHideInputBoxList.append(ui->CapDischarge);
        guestHideInputBoxList.append(ui->TCurCharge);
        guestHideInputBoxList.append(ui->TCurDischarge);
        guestHideInputBoxList.append(ui->TCharge);
        guestHideInputBoxList.append(ui->TDischarge);
        guestHideInputBoxList.append(ui->CountCharge);
        guestHideInputBoxList.append(ui->CountDischarge);
        guestHideInputBoxList.append(ui->CountLoopChgAndDis);
        guestHideInputBoxList.append(ui->CountDeepDisCharge);
        guestHideInputBoxList.append(ui->ERemain);
        guestHideInputBoxList.append(ui->ECurCharge);
        guestHideInputBoxList.append(ui->ECurDischarge);
        guestHideInputBoxList.append(ui->ECharge);
        guestHideInputBoxList.append(ui->EDischarge);
        guestHideInputBoxList.append(ui->SelfDisChargeRate);
        guestHideInputBoxList.append(ui->chgBalance);
        guestHideInputBoxList.append(ui->dischgBalance);
        guestHideInputBoxList.append(ui->CountSysProtect);
        guestHideInputBoxList.append(ui->CommStatus);
        guestHideInputBoxList.append(ui->can485Stat);
        guestHideInputBoxList.append(ui->RES002);


        ui->gp_filePort->setVisible(false);
        foreach(InputBox* obj, ui->tabSet->findChildren<InputBox*>())
        {
            obj->setEnabled(false);
        }

        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabSet));
        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabConfig));
        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabCtrl));
        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabBalance));
        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabBMUVer));
        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabSOE));

        ui->pb_clearNetErrCnt->setVisible(false);
        ui->sp_netErrCnt->setVisible(false);
    }
    else if(db_manager::Instance()->userLevel() == db_manager::LEVEL_SUPER)
    {
        ui->btn_debugLog->setVisible(false);
        ui->btnImportSOC->setVisible(false);

        ui->groupBoxDebugReg->setVisible(false);

        ui->gp_sysPara1->setVisible(true);
        ui->gp_sysPara2->setVisible(false);
        ui->gp_alarmValue1->setVisible(true);
        ui->gp_alarmValue2->setVisible(true);
        ui->gp_sysPara3->setVisible(false);
        ui->gp_muskBlock->setVisible(false);
        ui->gp_filePort->setVisible(true);
        ui->wd_fillLimit->setVisible(true);


        ui->gp_GuestSysStat->setVisible(false);

        ui->pb_modBusHelp->setVisible(true);

        foreach(InputBox* obj, ui->tabSet->findChildren<InputBox*>())
        {
            obj->setEnabled(true);
        }
        foreach(InputBox* obj, ui->gp_sysPara2->findChildren<InputBox*>())
        {
            obj->setEnabled(false);
        }
        foreach(InputBox* obj, ui->gp_sysPara3->findChildren<InputBox*>())
        {
            obj->setEnabled(false);
        }

        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabConfig));

        ui->G_FuncMask->setVisible(false);
        ui->G_sysCtrl->setVisible(true);
        ui->G_SensorCali->setVisible(true);
        ui->G_DeviceCtrl->setVisible(true);
        ui->G_DeviceDebug->setVisible(false);
        ui->wd_fillCtrl->setVisible(true);

        ui->G_FuncMask->setEnabled(true);
        ui->G_sysCtrl->setEnabled(true);
        ui->G_SensorCali->setEnabled(true);
        ui->G_DeviceCtrl->setEnabled(true);
        ui->G_DeviceDebug->setEnabled(false);

        ui->pb_clearNetErrCnt->setVisible(false);
        ui->sp_netErrCnt->setVisible(false);
    }
    else if(db_manager::Instance()->userLevel() == db_manager::LEVEL_DEBUG)
    {
        ui->btn_debugLog->setVisible(true);
        ui->btnImportSOC->setVisible(true);

        ui->groupBoxDebugReg->setVisible(true);

        ui->gp_GuestSysStat->setVisible(false);

        ui->gp_sysPara1->setVisible(true);
        ui->gp_sysPara2->setVisible(true);
        ui->gp_alarmValue1->setVisible(true);
        ui->gp_alarmValue2->setVisible(true);
        ui->gp_sysPara3->setVisible(true);
        ui->gp_muskBlock->setVisible(true);
        ui->gp_filePort->setVisible(true);
        ui->wd_fillLimit->setVisible(false);

        ui->pb_modBusHelp->setVisible(true);

        foreach(InputBox* obj, ui->tabSet->findChildren<InputBox*>())
        {
            obj->setEnabled(true);
        }

        ui->G_FuncMask->setVisible(true);
        ui->G_sysCtrl->setVisible(true);
        ui->G_SensorCali->setVisible(true);
        ui->G_DeviceCtrl->setVisible(true);
        ui->G_DeviceDebug->setVisible(true);
        ui->wd_fillCtrl->setVisible(false);


        ui->G_FuncMask->setEnabled(true);
        ui->G_sysCtrl->setEnabled(true);
        ui->G_SensorCali->setEnabled(true);
        ui->G_DeviceCtrl->setEnabled(true);
        ui->G_DeviceDebug->setEnabled(true);

        ui->DataWidget->removeTab(ui->DataWidget->indexOf(ui->tabConfig));

        ui->pb_clearNetErrCnt->setVisible(true);
        ui->sp_netErrCnt->setVisible(true);
    }

}

BMSView::BMSView(QWidget* parent) : QWidget(parent), ui(new Ui::BMSView) {
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &BMSView::timerUpDate);
    mycmu = nullptr;
    bmu_comm = 0;
    //    pmq = MessageQueue::getInstance();
    //    pmq->registMsgQueue(99);
    uiInit();
    load_config();
    config = {0, 0, 0, 0, 0};
    //
    QString protocol = QSettings("config.ini", QSettings::IniFormat).value("global/protocol", "CMU_V0").toString();


    ui->cbProtocol->blockSignals(true);
    ui->cbProtocol->clear();
    // 手动添加协议类型
    ui->cbProtocol->addItem("CMU_V0", CMU_V0);
    ui->cbProtocol->addItem("CMU_V1", CMU_V1);
    ui->cbProtocol->addItem("CMU_V2", CMU_V2);
    ui->cbProtocol->addItem("CMU_V3", CMU_V3);

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
    connect(this, &BMSView::send_msg, this->mycmu, &mb_cmu::msg_deal);
    connect(this->mycmu, &mb_cmu::connectChanged, this, [this](QString conn) { m_conn = conn; });
    connect(this->mycmu, &mb_cmu::cpVerChanged, this, [this](int ver){
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_SET_EXPRO;
        MsgCmd.data.setNum(ver);
        emit send_msg(MsgCmd);
        ui->cbProtocol->blockSignals(true);
        ui->cbProtocol->setCurrentIndex(ver/1000);
        ui->cbProtocol->blockSignals(false);
    });


    rebootbmus=new Rebootbmus(this);
    rebootbmus->setWindowFlags(Qt::Window); // 设置窗口标志
    connect(rebootbmus,&Rebootbmus::send_data,this,&BMSView::rebootBmus);
    TMsgData msg;
    msg.msg_type = CONFIG_INIT;
    emit send_msg(msg);
    timer->start(500);
    Toast::showTip(tr("初始化完成"), nullptr);

    uint errLogUcellLimit = settings->value("global/le_ErrLogUcellLimit", 100).toUInt();
    uint errLogTempLimit = settings->value("global/le_ErrLogTempLimit", 2).toUInt();
    uint errLogStdLimit = settings->value("global/le_ErrLog_StdValue", 1000).toUInt();

    ui->le_ErrLogUcellLimit->setText(QString::number(errLogUcellLimit));
    emit ui->le_ErrLogUcellLimit->editingFinished();
    ui->le_ErrLogTempLimit->setText(QString::number(errLogTempLimit));
    emit ui->le_ErrLogTempLimit->editingFinished();
    ui->le_ErrLog_StdValue->setText(QString::number(errLogStdLimit));
    emit ui->le_ErrLog_StdValue->editingFinished();

    connect(ui->rb_ErrLog_OnceWrite,&QRadioButton::toggled,this,&BMSView::radioBtnToggledChanged);
    connect(ui->rb_ErrLog_alwaysWrite,&QRadioButton::toggled,this,&BMSView::radioBtnToggledChanged);
    connect(ui->rb_ErrLog_NtimesWrite,&QRadioButton::toggled,this,&BMSView::radioBtnToggledChanged);
    connect(ui->sb_ErrLog_Count,&QSpinBox::textChanged,this,[=](){
        this->radioBtnToggledChanged(true);
    });

    lbListInit();

    initUserLevelForm();


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

void BMSView::StartBalanceForm()
{
    uint8_t mode = 0;
    mode = ui->BalnceMask->value();
    //        bool lbok;
    if (inputBalance == nullptr) {
        inputBalance = new frmBalanceBox();
        connect(inputBalance, &frmBalanceBox::valueChange, [this]() {
            TMsgData MsgCmd;
            uint16_t mode = inputBalance->getMode();
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t value[2] = {5408, mode};
            MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
            if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
            MsgCmd.data.clear();
            QByteArray ba = inputBalance->getValue();
            if (ba.size() > 0) {
                MsgCmd.msg_type = CTRL_AO_ADDR;
                MsgCmd.data.append(ba);
                if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
            }
        });
    }
    if(this->mycmu->is_pVer_a_fan_pal()){
        inputBalance->CMUVsersion = "CMUV4_6";
    }else{
        inputBalance->CMUVsersion = "Other";
    }
    inputBalance->setMode(mode);
    inputBalance->open();
    inputBalance->activateWindow();
}
BMSView::~BMSView() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    emit send_msg(MsgCmd);
    if (inputBalance) {
        inputBalance->close();
        inputBalance->deleteLater();
    }
    timer->stop();
    delete timer;
    delete ui;
}

void BMSView::uiChange(QHash<QString, qreal> mapData) {

    QStringList hdr_list;

    //tableW Volt
    hdr_list.clear();
    ui->tableBMU->setRowCount(config.bmu_num);

    if(db_manager::Instance()->userLevel() == db_manager::LEVEL_GUEST)
    {
        hdr_list.append(tr("BMU 软件版本号"));
    }
    for (int i = 0; i < config.vol_num; i++) {
        hdr_list.append(("Cell" + QString::number(i + 1)));
    }



    ui->tableBMU->setColumnCount(hdr_list.size());
    ui->tableBMU->setHorizontalHeaderLabels(hdr_list);
    ui->tableBMU->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableBMU->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个


    //tableW Temp
    ui->tableTemp->setRowCount(config.bmu_num);
    hdr_list.clear();
    for (int i = 0; i < config.T_num; i++) {
        if(i>=24){
            hdr_list.append((tr("铜排") + " T" + QString::number(i + 1)));
        }else{
            hdr_list.append(("T" + QString::number(i + 1)));
        }
    }
    for (int i = 0; i < config.Tp_num; i++) {
        hdr_list.append((tr("极柱") + " T" + QString::number(i + 1)));
    }
    ui->tableTemp->setColumnCount(hdr_list.size());
    ui->tableTemp->setHorizontalHeaderLabels(hdr_list);
    ui->tableTemp->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableTemp->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个


    //tableW Ver
    ui->tableVer->setRowCount(config.bmu_num);
    hdr_list.clear();
    hdr_list.append(tr("版本号"));
    hdr_list.append(tr("运行状态"));
    hdr_list.append(tr("故障状态"));
    if (this->mycmu->is_pVer_passive()) {
        if (this->mycmu->is_cpVer_Higher_than(CMU_P_V0_0_02)) {
            hdr_list.append(tr("CAN错误数"));
        }
        ui->BalnceStart->blockSignals(true);
        ui->BalnceStart->setObjectName("BalnceStart");
        ui->BalnceStart->setPrefix(tr("均衡启动阈值") + " ");
        ui->BalnceStart->setSuffix(" V");
        ui->BalnceStart->setMaximum(6);
        ui->BalnceStart->setToolTip(tr("均衡启动阈值"));
        ui->BalnceStart->blockSignals(false);
        ui->BalnceStart->setContextMenuPolicy(Qt::NoContextMenu);
    } else if (this->mycmu->is_pVer_active()) {
        if (this->mycmu->is_cpVer_with_fan_rate()) {
            hdr_list.append(tr("风机转速"));
        } else if(this->mycmu->is_pVer_a_liq_mos()){

        }else {
            hdr_list.append(tr("风机"));
        }

        hdr_list.append(tr("母线电压(V)"));
        if(this->mycmu->is_pVer_a_liq_mos()){
            hdr_list.append(tr("模组A均衡信息"));
            hdr_list.append(tr("模组B均衡信息"));
            hdr_list.append(tr("模组C均衡信息"));
            hdr_list.append(tr("模组D均衡信息"));
        }else{
            hdr_list.append(tr("均衡电流(A)"));
            hdr_list.append(tr("均衡故障"));
            hdr_list.append(tr("通道状态"));
        }
        hdr_list.append(tr("均衡模式"));
        hdr_list.append(tr("CAN错误数"));
        // 特殊处理
        ui->BalnceStart->blockSignals(true);
        ui->BalnceStart->setObjectName("BalanceConfig");
        ui->BalnceStart->setPrefix(tr("均衡配置") + " ");
        ui->BalnceStart->setSuffix("");
        ui->BalnceStart->setMaximum(100000);
        ui->BalnceStart->setToolTip(tr("均衡配置"));
        ui->BalnceStart->blockSignals(false);
        ui->BalnceStart->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    ui->tableVer->setColumnCount(hdr_list.size());
    ui->tableVer->setHorizontalHeaderLabels(hdr_list);
    ui->tableVer->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
    ui->tableVer->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个


    //tableW Ex
    if (this->mycmu->is_pVer_active()) {
        hdr_list.clear();
        ui->DataWidget->setTabEnabled(ui->DataWidget->indexOf(ui->tabBalance), true);

        if(this->mycmu->is_pVer_a_fan_pal()){
            hdr_list.append(tr("硬体版本"));
            hdr_list.append(tr("BOOT版本"));
            hdr_list.append(tr("生产流水号"));
            hdr_list.append(tr("模块温度1"));
            hdr_list.append(tr("模块温度2"));
        }else if(this->mycmu->is_cpVer_a_fan_mos_with_boot_ver()){
            hdr_list.append(tr("BOOT版本"));
        }else if(this->mycmu->is_pVer_a_liq_mos()){
            hdr_list.append(tr("BOOT版本"));
        }

        for (int i = 0; i < config.vol_num; i++) {
            hdr_list.append(tr("充电Ah") + QString::number(i + 1));
            hdr_list.append(tr("放电Ah") + QString::number(i + 1));
        }
        ui->tableExtView->setRowCount(config.bmu_num);
        ui->tableExtView->setColumnCount(hdr_list.size());
        ui->tableExtView->setHorizontalHeaderLabels(hdr_list);
        ui->tableExtView->setSelectionBehavior(QAbstractItemView::SelectItems);    // 单个选中
        ui->tableExtView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
    } else {
        ui->DataWidget->setTabEnabled(ui->DataWidget->indexOf(ui->tabBalance), false);
    }



    // 定值显示和隐藏
    QList<InputBox*> dspboxs = ui->tabSet->findChildren<InputBox*>();
    foreach (InputBox* dspbox, dspboxs) {
        dspbox->hide();
        if (mapData.contains(dspbox->objectName())) {
            dspbox->show();
        }
    }
    if(mapData.contains("SwitchONACPower")){
        ui->AllOnOffPower->show();
    }



    // 控制按钮显示与隐藏
    ui->AutoFindAddr->hide();    
    if(this->mycmu->is_pVer_a_liq_mos()){
        ui->AutoFindAddr->show();
    }

    ui->btnClrSysLock->hide();
    if(this->mycmu->is_cpVer_match(CMU_A_FAN_MOS_V1_0_02)
            || this->mycmu->is_cpVer_match(CMU_A_FAN_MOS_V1_0_04)
            || this->mycmu->is_exVer_3levels_alarm()
            || this->mycmu->is_pVer_a_liq_mos()){
        ui->btnClrSysLock->show();
    }


    setRegText();
//    if(this->mycmu->is_pVer_a_liq_mos()){

//    }else{

//    }



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
        emit send_msg(MsgCmd);

    } else {
        qDebug() << "can't find " << name;
    }
    return 0;
}
void BMSView::valueChange(double dval) {
    InputBox* b = qobject_cast<InputBox*>(sender());
    if (!b) return;
    if (myHelper::ShowMessageBoxQuesion(QString(tr("要修改\"%1\"为 %2 ?")).arg(b->prefix()).arg(dval)) !=
            QDialog::Accepted) {
        return;
    }
    b->clearFocus();
    //qDebug() << b->objectName() << ":" << dval;
    setValue(b->objectName(), dval);
}

void BMSView::timerUpDate() {
    QTime t;
    t.restart();  // 将此时间设置为当前时间
    //
    if (mycmu == nullptr) return;
    if (this->mycmu->drv_status) {
        ui->labelStatus->setStyleSheet("color:darkGreen");
        ui->labelStatus->setText(tr("已连接"));
        if (this->mycmu->drv_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("软件过期，请更新！"));
        uint32_t val = this->mycmu->cmu_ver;
        ui->btnVer->setText(QString("%1:%2").arg(tr("版本号"), myHelper::IntegerToHexString(val)));
//        ui->btnVer->setText(QString("%1:%2").arg(tr("版本号"), QString("%1.%2.%3.%4")
//                                                 .arg((val >> 24) & 0xFF, 2, 16, QChar('0'))
//                                                 .arg((val >> 16) & 0xFF, 2, 16, QChar('0'))
//                                                 .arg((val >> 8)  & 0xFF, 2, 16, QChar('0'))
//                                                 .arg((val >> 0)  & 0xFF, 2, 16, QChar('0')).toUpper()));
        findPreVer();
        ui->tbtnConnect->setText(tr("重连"));
        ui->tbtnConnect->setObjectName("reconnect");
    } else {
        if(ui->labelStatus->text() == tr("已连接"))
        {
            ui->sp_netErrCnt->setValue(ui->sp_netErrCnt->value()+1);
        }
        ui->labelStatus->setStyleSheet("color:;text-decoration:underline;font:bold;");
        ui->labelStatus->setText(tr("未连接"));
        ui->tbtnConnect->setText(tr("连接"));
        ui->tbtnConnect->setObjectName("connect");
    }
    this->setWindowTitle(QString("%1[%2]").arg(m_conn, ui->labelStatus->text()));
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
    // qDebug() << t.elapsed() << "ms";
}
QString BMSView::getBmuErrInfo(uint16_t value, QString tips) {
    QStringList statusList;
    QStringList tipList;

    if(this->mycmu->is_pVer_a_fan_mos())
    {
        for(int i = 0; i < 16; i++)
        {
            if (GET_BIT(value, i))
            {
                statusList << this->mycmu->BmuErrStr_a[i];
            }
        }
    }
    else if(this->mycmu->is_pVer_a_fan_pal())
    {
        for(int i = 0; i < 16; i++)
        {
            if (GET_BIT(value, i))
            {
                statusList << this->mycmu->BmuErrStr_m[i];
            }
        }
    }
    else if(this->mycmu->is_pVer_a_liq_mos())
    {
        for(int i = 0; i < 16; i++)
        {
            if (GET_BIT(value, i))
            {
                statusList << this->mycmu->BmuErrStr_w[i];
            }
        }
    }
    else
    {
        tipList = QStringList({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"});
        for (int i = 0; i < tipList.size(); i++) {
            if ((((value >> i) & 0x01) > 0)) statusList << tipList.at(i);
        }
    }
    return statusList.join("|");
}


void BMSView::statGroupAutoHide(QHash<QString, qreal> mapData)
{
    if(db_manager::Instance()->userLevel() == db_manager::LEVEL_GUEST)
    {
        ui->G_SysStatus->setVisible(false);
        ui->G_DIStatus->setVisible(false);
        ui->G_DOStatus->setVisible(false);
        ui->G_ErrStatus->setVisible(false);
        ui->G_AlmStatus->setVisible(false);
        ui->G_PreAlmStatus->setVisible(false);
    }
    else
    {
        if (!(mapData.contains("sysStatus")||mapData.contains("sysStatus2")))
        {
            ui->G_SysStatus->setVisible(false);
        }
        else
        {
            ui->G_SysStatus->setVisible(true);
        }

        if (!(mapData.contains("sysErrStatus")||mapData.contains("sysErrStatus2")))
        {
            ui->G_ErrStatus->setVisible(false);
        }
        else
        {
            ui->G_ErrStatus->setVisible(true);
        }

        if (!(mapData.contains("sysAlmStatus")||mapData.contains("sysAlmStatus2")))
        {
            ui->G_AlmStatus->setVisible(false);
        }
        else
        {
            ui->G_AlmStatus->setVisible(true);
        }

        if (!(mapData.contains("sysPreAlmStatus")))
        {
            ui->G_PreAlmStatus->setVisible(false);
        }
        else
        {
            ui->G_PreAlmStatus->setVisible(true);
        }

        if (!(mapData.contains("sysDIStatus")))
        {
            ui->G_DIStatus->setVisible(false);
        }
        else
        {
            ui->G_DIStatus->setVisible(true);
        }

        if (!(mapData.contains("sysDOStatus")))
        {
            ui->G_DOStatus->setVisible(false);
        }
        else
        {
            ui->G_DOStatus->setVisible(true);
        }
    }
}

void BMSView::setDoButtonText(uint16_t value, QList<QString> textList)
{
    ui->btnDebugRelayCtrl0On->setVisible(false);
    foreach(QPushButton * btn, ui->G_DeviceDebug->findChildren<QPushButton*>())
    {
        QString objName = btn->objectName();
        if(objName.startsWith("btnDebugRelayCtrl"))
        {
            objName.replace("btnDebugRelayCtrl","");
            QString ctrl = "";

            if(objName.endsWith("On"))
            {
                ctrl = tr("闭合");
            }
            else if(objName.endsWith("Off"))
            {
                ctrl = tr("断开");
            }

            objName.replace("On","").replace("Off","");

            int idx = objName.toInt();
            if(idx >= 0 || idx < 16)
            {
                if(ctrl == tr("闭合") || ctrl == tr("断开"))
                {
                    QString str = textList.at(idx);

                    if(str == RESERVED_TEXT_RES)
                    {
                        str = "DO-" + QString::number(idx);
                    }

                    str += ctrl;
                    btn->setText(str);
                }
            }

        }
        else if(objName.startsWith("btnDoStat"))
        {
            objName.replace("btnDoStat","");
            int idx = objName.toInt();
            if(idx >= 0 || idx < 16)
            {
                bool flag = ((value >> idx) & 0x01) > 0;
                QString color = flag ? BTN_RED : BTN_GREEN;
                btn->setStyleSheet(QString("%1").arg(color));
            }

        }

    }


}

void BMSView::setRegText()
{
    ui->DataBlock1->setPrefix("REG5422");
    ui->DataBlock2->setPrefix("REG5423");
    ui->DataBlock3->setPrefix("REG5424");
    ui->DataBlock4->setPrefix("REG5425");

    ui->FuncMask->setPrefix("REG5417");
    ui->FuncMask2->setPrefix("REG5430");
    ui->ClusterAlmMask->setPrefix("REG5415");
    ui->ClusterErrMask->setPrefix("REG5416");

    ui->ErrBlock1->setPrefix("REG5426");
    ui->ErrBlock2->setPrefix("REG5427");
    ui->WarnBlock1->setPrefix("REG5428");
    ui->WarnBlock2->setPrefix("REG5429");

    if(this->mycmu->is_pVer_a_liq_mos())
    {
        ui->UBlock->setPrefix("REG5432");
        ui->UBlock2->setPrefix("REG5433");
        ui->UBlock3->setPrefix("REG5434");
        ui->UBlock4->setPrefix("REG5435");
        ui->TBlock->setPrefix("REG5436");
        ui->TBlock2->setPrefix("REG5437");
        ui->TBlock3->setPrefix("REG5438");
        ui->TBlock4->setPrefix("REG5439");
        qDebug() << "change ui reg name ----> liq cool tab form";
    }
    else
    {
        ui->UBlock->setPrefix("REG5432");
        ui->TBlock->setPrefix("REG5433");
        qDebug() << "change ui reg name ----> fan cool tab form";
    }
}

void BMSView::flushData(int type, QHash<QString, qreal> mapData) {

    QElapsedTimer flushDataTimeCostMs;
    flushDataTimeCostMs.start();



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

    // 刷新定值
    QList<InputBox*> inputs = ui->tabSet->findChildren<InputBox*>();
    foreach (InputBox* dspbox, inputs) {
        if (mapData.contains(dspbox->objectName())) {
            if (dspbox->hasFocus()) continue;
            dspbox->blockSignals(true);
            dspbox->setValueDirect(mapData.value(dspbox->objectName()));
            dspbox->blockSignals(false);
        } else {
        }
    }

    QList<InputBox*> dspboxs = ui->tabCMU->findChildren<InputBox*>();
    foreach (InputBox* dspbox, dspboxs) {
        if(db_manager::Instance()->userLevel() == db_manager::LEVEL_GUEST)
        {
            if(guestHideInputBoxList.contains(dspbox))
            {
                dspbox->hide();
            }
            else
            {
                if (mapData.contains(dspbox->objectName()))
                {
                    dspbox->show();
                    if(dspbox->objectName().endsWith("ID"))
                    {
                        dspbox->setText(myHelper::IDToString(mapData.value(dspbox->objectName()), config.vol_num));
                    }
                    else
                    {
                        dspbox->setValue(mapData.value(dspbox->objectName()));
                    }
                }
                else
                {
                    dspbox->hide();
                }
            }
        }
        else
        {
            if (mapData.contains(dspbox->objectName()))
            {

                dspbox->show();
                if(dspbox->objectName().endsWith("ID"))
                {
                    dspbox->setText(myHelper::IDToString(mapData.value(dspbox->objectName()), config.vol_num));
                }
                else
                {
                    dspbox->setValue(mapData.value(dspbox->objectName()));
                }

            }
            else
            {
                dspbox->hide();
            }
        }

    }

    if (mapData.contains("sysComm2")) {
        ui->CommStatus->show();
        uint32_t comm_status1 = mapData.value("sysComm1", 0);
        uint32_t comm_status2 = mapData.value("sysComm2", 0);

        bmu_comm = ((uint64_t)comm_status2 << 32) | comm_status1;
        uint64_t bmuOffLine = ~bmu_comm;
//        qDebug() << config.bmu_num << QString("comm2 0x%1, comm1 0x%2, comm 0x%3")
//                    .arg(comm_status2,8,16,QChar('0'))
//                    .arg(comm_status1,8,16,QChar('0'))
//                    .arg(bmu_comm,16,16,QChar('0'));
        QString offLineId = "";
        for(int i = 0; i < config.bmu_num; i++)
        {
            if(bmuOffLine&(uint64_t(0x01)<<i))
            {
                offLineId += QString::number(i+1)+ " ";
            }
        }
        ui->CommStatus->setText(offLineId);
        if(offLineId == "")
        {
            ui->CommStatus->setToolTip(tr("none"));
        }
        else
        {
            ui->CommStatus->setToolTip(offLineId);
        }
    }
    if(mapData.contains("can485Stat")){
        uint32_t commStStat = mapData.value("can485Stat", 0);
        ui->can485Stat->setText(QString("0x%1").arg(commStStat,4,16,QChar('0')));
    }
    if (mapData.contains("sysTime")) {
        ui->sysTime->setValue(mapData.value("sysTime"));
    }



    pcsPowerMapDataCache.clear();
    if(mapData.contains("SwitchONACPower")){
        ui->AllOnOffPower->show();
        if(!mapData.contains("NoAcPCSOnPower")){
            pcsPowerMapDataCache = mapData;
            uint16_t value1 = mapData.value("SwitchONACPower");
            uint16_t value2 = mapData.value("SwitchPower");
            uint16_t value3 = mapData.value("SwitchOFFPower");
//            qDebug() << value1 << value2 << value3;
            ui->AllOnOffPower->setText(QString("0x%1 0x%2 0x%3")
                                       .arg(value1,4,16,QChar('0'))
                                       .arg(value2,4,16,QChar('0'))
                                       .arg(value3,4,16,QChar('0')));
        }else{
            pcsPowerMapDataCache = mapData;
            uint16_t value1 = mapData.value("SwitchONACPower");
            uint16_t value2 = mapData.value("SwitchPower");
            uint16_t value3 = mapData.value("SwitchOFFPower");
            uint16_t value4 = mapData.value("NoAcPCSOnPower");
            uint16_t value5 = mapData.value("NoAcPCSOffPower");
            ui->AllOnOffPower->setText(QString("0x%1 0x%2 0x%3 0x%4 0x%5")
                                       .arg(value1,4,16,QChar('0'))
                                       .arg(value2,4,16,QChar('0'))
                                       .arg(value3,4,16,QChar('0'))
                                       .arg(value4,4,16,QChar('0'))
                                       .arg(value5,4,16,QChar('0')));
//            qDebug() << value1 << value2 << value3 << value4 << value5 ;
        }

    }else{
        ui->AllOnOffPower->hide();
    }

    if (mapData.contains("sysStatus1")) {
        uint16_t value = mapData.value("sysStatus1");
        uint16_t valueApend = mapData.value("sysStatus2");
        ui->G_SysStatus->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("系统状态"))
                                  .arg(value,4,16,QChar('0'))
                                  .arg(valueApend,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义

        QStringList textList;
        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("CMU总故障") << tr("CMU总告警") << tr("电池充满") << tr("电池放空")
                     << tr("系统未初始化") << RESERVED_TEXT_RES << tr("均衡状态") << tr("电池充电")
                     << tr("电池放电") << tr("系统停机") << tr("升级标志") << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << tr("BMU拨码异常") << tr("CMU总预警") << tr("并网状态");
        }else{
            textList << tr("CMU总故障") << tr("CMU总告警") << tr("电池充满") << tr("电池放空")
                     << tr("系统未初始化") << tr("CMU-BMU通信异常") << tr("均衡状态") << tr("电池充电")
                     << tr("电池放电") << tr("系统停机") << tr("升级标志") << tr("CMU-INS通信异常")
                     << tr("自检") << tr("BMU拨码异常") << tr("BMU故障") << tr("并网状态");
        }
        fillStatLabel(lbSysStatList, value, textList);
    }

    if (mapData.contains("sysStatus2")) {
        uint16_t value = mapData.value("sysStatus2");
        uint16_t valueApend = mapData.value("sysStatus1");
        ui->G_SysStatus->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("系统状态"))
                                  .arg(valueApend,4,16,QChar('0'))
                                  .arg(value,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义

        QStringList textList;
        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("IO解锁") << tr("绝缘使能") << tr("RTU风扇使能") << tr("RTU核容使能")
                     << tr("PCS运行状态") << tr("SOC需要校准") << tr("CMU总故障锁定") << EMPTY_TEXT
                     << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT
                     << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT;
        }else{
            textList << tr("IO解锁") << tr("绝缘使能") << tr("RTU风扇使能") << tr("RTU核容使能")
                     << tr("PCS运行状态") << tr("SOC需要校准") << tr("CMU总故障锁定") << EMPTY_TEXT
                     << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT
                     << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT << EMPTY_TEXT;
        }
        fillStatLabel(lbSysStat2List, value, textList);
    }else{
        clearStatLabel(lbSysStat2List);
    }

    if (mapData.contains("sysErrStatus")) {
        uint16_t value = mapData.value("sysErrStatus");
        uint16_t valueApend = mapData.value("sysErrStatus2");
        ui->G_ErrStatus->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("故障状态"))
                                  .arg(value,4,16,QChar('0'))
                                  .arg(valueApend,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义
//        ui->G_ErrStatus->setTitle(QString("%1(%2)").arg(tr("故障状态") + "1").arg(value));

        QStringList textList;
        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("电芯过压故障") << tr("电芯欠压故障") << tr("电芯高温故障") << tr("电芯低温故障")
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES<< tr("Pack极柱高温故障") << tr("充放电过流故障")
                     << tr("簇短路故障") << tr("簇过压故障") << tr("簇欠压故障") << tr("簇绝缘故障")
                     << tr("簇漏电故障") << tr("HVU极柱高温故障") << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else{
            textList << tr("电芯过压故障") << tr("电芯欠压故障") << tr("电芯高温故障") << tr("电芯低温故障")
                     << tr("电芯温升故障") << tr("温差过大故障") << tr("Pack极柱高温故障") << tr("充放电过流故障")
                     << tr("簇短路故障") << tr("簇过压故障") << tr("簇欠压故障") << tr("簇绝缘故障")
                     << tr("簇漏电故障")<< tr("HVU极柱高温故障") << tr("SOC过低故障") << tr("压差过大故障");
        }

        fillStatLabel(lbErrStatList, value, textList);
    }

    if (mapData.contains("sysErrStatus2")) {
        uint16_t value = mapData.value("sysErrStatus2");
        uint16_t valueApend = mapData.value("sysErrStatus");
        ui->G_ErrStatus->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("故障状态"))
                                  .arg(valueApend,4,16,QChar('0'))
                                  .arg(value,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义
//        ui->G_ErrStatus->setTitle(QString("%1(%2)").arg(tr("故障状态") + "2").arg(value));

        QStringList textList;
        if(this->mycmu->is_cpVer_match(CMU_A_FAN_MOS_V1_3_00)){
            textList << tr("烟感故障") << tr("水浸故障") << tr("消防故障") << tr("急停故障")
                     << tr("电芯过压锁定") << tr("电芯欠压锁定")<< tr("充放电过流锁定") << tr("电芯高温锁定")
                     << tr("电芯低温锁定") << tr("Pack极柱高温锁定") << tr("HVU极柱高温锁定") << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else if(this->mycmu->is_cpVer_match(CMU_A_LIQ_MOS_V3_3_00)){
            textList << tr("烟感故障") << tr("水浸故障") << tr("消防故障") << tr("急停故障")
                     << tr("电芯过压锁定") << tr("电芯欠压锁定")<< tr("充放电过流锁定") << tr("电芯高温锁定")
                     << tr("电芯低温锁定") << tr("Pack极柱高温锁定") << tr("HVU极柱高温锁定") << tr("铜排高温保护")
                     << tr("铜排低温保护") << tr("铜排高温锁定") << tr("铜排低温锁定") << RESERVED_TEXT_RES;
        }else{
            textList << tr("烟感故障") << tr("水浸故障") << tr("消防故障") << tr("急停故障")
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES<< RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }

        fillStatLabel(lbErrStat2List, value, textList);
    }else{
        clearStatLabel(lbErrStat2List);
    }

    if (mapData.contains("sysAlmStatus")) {
        uint16_t value = mapData.value("sysAlmStatus");
        uint16_t valueApend = mapData.value("sysAlmStatus2");
        ui->G_AlmStatus->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("告警状态"))
                                  .arg(value,4,16,QChar('0'))
                                  .arg(valueApend,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义
//        ui->G_AlmStatus->setTitle(QString("%1(%2)").arg(tr("告警状态") + "1").arg(value));

        QStringList textList;
        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("电芯过压告警") << tr("电芯欠压告警") << tr("电芯高温告警") << tr("电芯低温告警")
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES<< tr("Pack极柱高温告警") << tr("充放电过流告警")
                     << tr("BMU异常告警") << tr("簇过压告警") << tr("簇欠压告警") << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << tr("HVU极柱高温告警") << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else{
            textList << tr("电芯过压告警") << tr("电芯欠压告警") << tr("电芯高温告警") << tr("电芯低温告警")
                     << tr("电芯温升告警") << tr("温差过大告警")<< tr("Pack极柱高温告警") << tr("充放电过流告警")
                     << tr("BMU异常告警") << tr("簇过压告警") << tr("簇欠压告警") << tr("绝缘下降告警")
                     << tr("漏电流告警") << tr("簇极柱高温告警") << tr("SOC过低告警") << tr("压差过大告警");
        }

        fillStatLabel(lbAlmStatList, value, textList);
    }

    if (mapData.contains("sysAlmStatus2")) {
        uint16_t value = mapData.value("sysAlmStatus2");
        uint16_t valueApend = mapData.value("sysAlmStatus");
        ui->G_AlmStatus->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("告警状态"))
                                  .arg(valueApend,4,16,QChar('0'))
                                  .arg(value,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义
//        ui->G_AlmStatus->setTitle(QString("%1(%2)").arg(tr("告警状态") + "2").arg(value));

        QStringList textList;
        if(this->mycmu->is_cpVer_match(CMU_A_FAN_MOS_V1_3_00)){
            textList << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << tr("电流采样异常告警")<< tr("断路器拒动告警") << tr("接触器拒动告警")
                     << tr("Pack气溶胶动作告警") << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else if(this->mycmu->is_cpVer_match(CMU_A_LIQ_MOS_V3_3_00)){
            textList << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << tr("电流采样异常告警")<< tr("断路器拒动告警") << tr("接触器拒动告警")
                     << tr("气溶胶告警") << RESERVED_TEXT_RES << tr("铜排高温告警") << tr("铜排低温告警")
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else if(this->mycmu->is_pVer_active()){
            textList << tr("BMU拨码异常告警") << tr("电压线束断线告警") << tr("温度线束断线告警") << tr("簇极柱温度断线告警")
                     << tr("电压传感器断线告警") << tr("预留传感器断线告警")<< tr("断路器拒动告警") << tr("接触器拒动告警")
                     << tr("簇压差过大告警") << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else{
            textList << tr("BMU拨码异常告警") << tr("电压线束断线告警") << tr("温度线束断线告警") << tr("簇极柱温度断线告警")
                     << tr("电压传感器断线告警") << tr("预留传感器断线告警")<< tr("断路器拒动告警") << tr("充放电过流告警")
                     << tr("BMU异常告警") << tr("簇过压告警") << tr("簇欠压告警") << tr("接触器拒动告警")
                     << tr("绝缘板采样压差过大告警") << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }
        fillStatLabel(lbAlmStat2List, value, textList);
    }else{
        clearStatLabel(lbAlmStat2List);
    }

    if (mapData.contains("sysPreAlmStatus")) {
        ui->G_PreAlmStatus->setVisible(true);
        uint16_t value = mapData.value("sysPreAlmStatus");
        ui->G_PreAlmStatus->setTitle(QString("%1: (0x%2)")
                                  .arg(tr("预警状态"))
                                  .arg(value,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义
//        ui->G_PreAlmStatus->setTitle(QString("%1(%2)").arg(tr("预警状态") + "1").arg(value));

        QStringList textList;
        if(this->mycmu->is_cpVer_match(CMU_A_FAN_MOS_V1_3_00)){
            textList << tr("电芯电压采样异常") << tr("电芯温度采样异常") << tr("Pack极柱温度采样异常") << tr("HVU极柱温度采样异常")
                     << tr("簇总压采样异常") << tr("电芯压差异常")<< tr("电芯温差异常") << tr("簇总压压差异常")
                     << tr("均衡功能异常") << tr("CMU-BMU通信异常") << tr("CMU-INS通信异常") << tr("CAN霍尔信号异常")
                     << tr("AI霍尔信号异常") << "电芯电压更新异常" << tr("自动寻址异常") << "BMU均衡采样异常";
        }else if(this->mycmu->is_cpVer_match(CMU_A_LIQ_MOS_V3_3_00)){
            textList << tr("电芯电压采样异常") << tr("电芯温度采样异常") << tr("Pack极柱温度采样异常") << tr("HVU极柱温度采样异常")
                     << tr("簇总压采样异常") << tr("电芯压差异常")<< tr("电芯温差异常") << tr("簇总压压差异常")
                     << tr("均衡功能异常") << tr("CMU-BMU通信异常") << tr("CMU-INS通信异常") << tr("CAN霍尔信号异常")
                     << tr("AI霍尔信号异常") << tr("电芯电压更新异常") << tr("自动寻址异常") << "BMU均衡采样异常";
        }else{
            textList << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }
        fillStatLabel(lbWarmStatList, value, textList);
    }else{
        clearStatLabel(lbWarmStatList);
    }

    if (mapData.contains("sysDIStatus")) {
        uint16_t value = mapData.value("sysDIStatus");
        ui->G_DIStatus->setTitle(QString("%1: (0x%2)")
                                  .arg(tr("DI状态"))
                                  .arg(value,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义

        QStringList textList;
        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("断路器QF状态") << tr("接触器KM+状态") << tr("接触器KM-状态") << tr("接触器KMR状态")
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << tr("交流有压状态")
                     << tr("黑启动控制状态") << tr("外部硬线跳QF状态") << tr("总故障继电器状态") << tr("BMU风扇继电器状态")
                     << tr("HVU风扇继电器状态") << tr("充满继电器状态") << tr("放空继电器状态") << RESERVED_TEXT_RES;
        }else{
            textList << tr("断路器QF状态") << tr("接触器KM+状态") << tr("接触器KM-状态") << tr("接触器KMR状态")
                     << tr("故障输入") << tr("主从状态") << tr("预留(水浸)") << tr("交流有压状态")
                     << tr("急停故障") << tr("QF继电器状态") << tr("总故障继电器状态") << tr("BMU风扇继电器状态")
                     << tr("HVU风扇继电器状态") << tr("充满继电器状态") << tr("放空继电器状态") << RESERVED_TEXT_RES;
        }

        fillStatLabel(lbDIStatList, value, textList);
    }

    if (mapData.contains("sysDOStatus")) {
        uint16_t value = mapData.value("sysDOStatus");
        ui->G_DOStatus->setTitle(QString("%1: (0x%2)").arg(tr("DO状态")).arg(value,4,16,QChar('0')));

        QStringList textList;
        if(this->mycmu->is_pVer_a_liq_mos()){
            textList << tr("QF输出") << tr("KM+输出") << tr("KM-输出") << tr("KMR输出")
                     << tr("故障输出") << tr("充电指示") << tr("放电指示") << tr("系统运行")
                     << tr("自动寻址信号") << tr("告警输出") << tr("Pack风扇电源") << tr("Hvu风扇电源")
                     << tr("充满输出") << tr("放空输出") << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }else{
            textList << tr("QF输出") << tr("KM+输出") << tr("KM-输出") << tr("KMR输出")
                     << tr("故障输出") << tr("充电指示") << tr("放电指示") << tr("系统运行")
                     << tr("BMU供电") << tr("告警输出") << tr("风扇电源输出") << RESERVED_TEXT_RES
                     << tr("充满输出") << tr("放空输出") << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
        }

        fillStatLabel(lbDOStatList, value, textList);
        setDoButtonText(value, textList);
    }

    if (mapData.contains("FuncMask")) {
        uint16_t value = mapData.value("FuncMask");
        uint16_t valueApend = mapData.value("FuncMask2");
        ui->func1Bit->setVisible(true);
        ui->func1Bit->setText(QString("0x%1")
                              .arg(value,4,16,QChar('0')));
        ui->G_FuncMask->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("使能位"))
                                  .arg(value,4,16,QChar('0'))
                                  .arg(valueApend,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义

//        ui->G_FuncMask->setTitle(QString("%1(%2)").arg(tr("使能位")).arg(value));
        QList<QCheckBox*> CheckBoxList;
        CheckBoxList << ui->bFunc00 << ui->bFunc01 << ui->bFunc02 << ui->bFunc03 << ui->bFunc04 << ui->bFunc05 << ui->bFunc06
                     << ui->bFunc07 << ui->bFunc08 << ui->bFunc09 << ui->bFunc10 << ui->bFunc11 << ui->bFunc12
                     << ui->bFunc13 << ui->bFunc14 << ui->bFunc15;
        QStringList textList;

        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("使能Ain电流传感器") << tr("使能绝缘板测总电压") << tr("禁用预留电流传感器") << tr("使能漏电流传感器")
                     << tr("使能绝缘检测") << tr("开启MODBUS写保护") << tr("使能故障录波功能") << tr("关闭参数设置限值") << tr("使能本地环控")
                     << tr("关闭接触器远程控制") << tr("调试信息UDP输出") << tr("使能调试信息输出") << tr("使能SOC校准")
                     << tr("解列模式") << tr("禁用旧版预充策略") << tr("使能外部安防控制");
        }
        else if(this->mycmu->is_pVer_active()){
            textList << tr("使能Ain电流传感器") << tr("使能绝缘板测总电压") << tr("禁用预留电流传感器") << tr("使能漏电流传感器")
                     << tr("使能绝缘检测") << tr("开启MODBUS写保护") << tr("使能故障录波功能") << tr("关闭参数设置限值") << tr("使能本地环控")
                     << tr("关闭接触器远程控制") << tr("调试信息UDP输出") << tr("使能调试信息输出") << tr("使能SOC校准")
                     << tr("解列模式") << tr("禁用旧版预充策略") << tr("使能外部安防控制");
        }
        else
        {
            textList << tr("使能双CAN") << tr("使能电流传感器") << tr("使能电压传感器") << tr("使能漏电流传感器")
                     << tr("使能绝缘检测") << tr("开启MODBUS写保护") << tr("使能故障录波功能") << tr("关闭参数设置限值") << tr("使能本地环控")
                     << tr("关闭接触器远程控制") << tr("调试信息UDP输出") << tr("使能调试信息输出") << tr("单簇/多簇")
                     << tr("解列模式") << tr("禁用旧版预充策略") << tr("使能外部安防控制");
        }
        foreach (QCheckBox* cb, CheckBoxList) {
            cb->blockSignals(true);
            cb->setChecked(((value >> CheckBoxList.indexOf(cb)) & 0x01) > 0);
            cb->blockSignals(false);
            cb->setText(textList.at(CheckBoxList.indexOf(cb)));
        }
    }

    if (mapData.contains("FuncMask2")) {
        uint16_t value = mapData.value("FuncMask2");
        uint16_t valueApend = mapData.value("FuncMask");
        ui->func2Bit->setVisible(true);
        ui->func2Bit->setText(QString("0x%1")
                              .arg(value,4,16,QChar('0')));
        ui->G_FuncMask->setTitle(QString("%1: (0x%2)(0x%3)")
                                  .arg(tr("使能位"))
                                  .arg(valueApend,4,16,QChar('0'))
                                  .arg(value,4,16,QChar('0')));//布局不一定是按照点表来的，不一定有意义

        QList<QCheckBox*> CheckBoxList;
        CheckBoxList << ui->bFuncEx00 << ui->bFuncEx01 << ui->bFuncEx02 << ui->bFuncEx03 << ui->bFuncEx04 << ui->bFuncEx05 << ui->bFuncEx06
                     << ui->bFuncEx07 << ui->bFuncEx08 << ui->bFuncEx09 << ui->bFuncEx10 << ui->bFuncEx11 << ui->bFuncEx12
                     << ui->bFuncEx13 << ui->bFuncEx14 << ui->bFuncEx15;
        QStringList textList;

        int tempInt = 0;
        tempInt = (ui->bFuncEx13->isChecked()?1:0) * 2 + (ui->bFuncEx12->isChecked()?1:0) * 1;
        ui->cB_Func2_1213->setCurrentIndex(tempInt);

        tempInt = (ui->bFuncEx15->isChecked()?1:0) * 2 + (ui->bFuncEx14->isChecked()?1:0) * 1;
        ui->cB_Func2_1415->setCurrentIndex(tempInt);

        if(this->mycmu->is_exVer_3levels_alarm()){
            textList << tr("使能全程投入绝缘检测") << tr("使能常规均衡策略") << tr("开启恒压充电模式") << tr("使能SOC均衡调整模式")
                     << tr("使能SOC计算阈值0.2A") << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << tr("SOC选择L")
                     << tr("SOC选择H") << tr("电池选择L") << tr("电池选择H");
            ui->label_bark1->setVisible(true);
            ui->label_bark2->setVisible(true);
            ui->cB_Func2_1213->setVisible(true);
            ui->cB_Func2_1415->setVisible(true);

            ui->cB_Func2_1213->setItemText(0,tr("0-Mode1"));
            ui->cB_Func2_1213->setItemText(1,tr("1-Mode2"));
            ui->cB_Func2_1213->setItemText(2,tr("2-Mode3"));
            ui->cB_Func2_1213->setItemText(3,tr("3-预留"));

            ui->cB_Func2_1415->setItemText(0,tr("0-90Ah"));
            ui->cB_Func2_1415->setItemText(1,tr("1-280Ah"));
            ui->cB_Func2_1415->setItemText(2,tr("2-306Ah"));
            ui->cB_Func2_1415->setItemText(3,tr("3-预留"));
        }else if(this->mycmu->is_pVer_active()){
            textList << tr("使能全程投入绝缘检测") << tr("使能常规均衡策略") << tr("开启恒压充电模式") << tr("使能SOC均衡调整模式")
                     << tr("使能SOC计算阈值0.2A") << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << tr("SOC选择L")
                     << tr("SOC选择H") << tr("电池选择L") << tr("电池选择H");
            ui->label_bark1->setVisible(true);
            ui->label_bark2->setVisible(true);
            ui->cB_Func2_1213->setVisible(true);
            ui->cB_Func2_1415->setVisible(true);

            ui->cB_Func2_1213->setItemText(0,tr("0-Mode1"));
            ui->cB_Func2_1213->setItemText(1,tr("1-Mode2"));
            ui->cB_Func2_1213->setItemText(2,tr("2-Mode3"));
            ui->cB_Func2_1213->setItemText(3,tr("3-预留"));

            ui->cB_Func2_1415->setItemText(0,tr("0-90Ah"));
            ui->cB_Func2_1415->setItemText(1,tr("1-280Ah"));
            ui->cB_Func2_1415->setItemText(2,tr("2-306Ah"));
            ui->cB_Func2_1415->setItemText(3,tr("3-预留"));
        }else{
            textList << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES
                     << RESERVED_TEXT_RES << RESERVED_TEXT_RES << RESERVED_TEXT_RES;
                ui->label_bark1->setVisible(false);
                ui->label_bark2->setVisible(false);
                ui->cB_Func2_1213->setVisible(false);
                ui->cB_Func2_1415->setVisible(false);

                ui->cB_Func2_1213->setItemText(0,tr("0-预留"));
                ui->cB_Func2_1213->setItemText(1,tr("1-预留"));
                ui->cB_Func2_1213->setItemText(2,tr("2-预留"));
                ui->cB_Func2_1213->setItemText(3,tr("3-预留"));

                ui->cB_Func2_1415->setItemText(0,tr("0-预留"));
                ui->cB_Func2_1415->setItemText(1,tr("1-预留"));
                ui->cB_Func2_1415->setItemText(2,tr("2-预留"));
                ui->cB_Func2_1415->setItemText(3,tr("3-预留"));
        }

        foreach (QCheckBox* cb, CheckBoxList) {
            cb->blockSignals(true);
            cb->setChecked(((value >> CheckBoxList.indexOf(cb)) & 0x01) > 0);
            cb->blockSignals(false);
            cb->setText(textList.at(CheckBoxList.indexOf(cb)));
        }
    }



    statGroupAutoHide(mapData);

    if (mapData.contains("BalnceMask") && mapData.contains("BalanceConfig")) {
        ui->balanceStr->show();
        uint16_t mode = mapData.value("BalnceMask");
        uint16_t value = mapData.value("BalanceConfig");
        QString str;
        if(this->mycmu->is_pVer_a_liq_mos())
        {
            str = QString(tr("%1对,%2A,%3秒")).arg(value >> 11).arg((value >> 8) & 0x7).arg(value & 0xFF);
        }
        else
        {
            str = QString(tr("%1对,%2A,%3秒")).arg(value >> 12).arg((value >> 8) & 0xF).arg(value & 0xFF);
        }

        switch (mode) {
        case 0x00:
            str = QString("%1:%2").arg(tr("禁止均衡"), str);
            break;
        case 0x55:
            str = QString("%1:%2").arg(tr("强制均衡"), str);
            break;
        case 0xAA:
            str = QString("%1:%2").arg(tr("自动均衡"), str);
            break;
        case 0x88:
            str = QString("%1:%2").arg(tr("手动均衡"), str);
            break;
        default:
            str = QString("%1:%2").arg(tr("未定义"), str);
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
        {
            QString tftpServerIp = myHelper::IPV4IntegerToString(mapData.value("ServIP"));
            ui->lineEditServIP->setText(tftpServerIp);
            //tftp server ip change
            if(myHelper::IsIP(tftpServerIp) && (tftpServerIp != "0.0.0.0"))
            {
                qInfo() << "send new tftp server ip:" << tftpServerIp;
                emit tftpServerIpChanged(tftpServerIp);
            }

        }
    }
    if (!ui->lineEditIP->hasFocus()) {
        if (mapData.contains("LocalIP"))
            ui->lineEditIP->setText(myHelper::IPV4IntegerToString(mapData.value("LocalIP")));
    }



    //没有用到的校准按钮
    ui->btnIZeroAdj->setVisible(false);
    ui->btnIFullAdj->setVisible(false);

    ui->sysStatWd->refreashAllStat();

    //qDebug() << "<><><><>flush data type " << type << "cost time" << flushDataTimeCostMs.elapsed() << "ms";
}

QString BMSView::getBmuInfo(uint16_t status) {
    QStringList statusList;

    if(this->mycmu->is_pVer_a_fan_mos())
    {
        for(int i = 0; i < 8; i++)
        {
            if (GET_BIT(status, i))
            {
                statusList << this->mycmu->BmuStateStr_a[i];
            }
            else
            {
                statusList << this->mycmu->BmuStateStr_a[i + 8];
            }
        }
        statusList << "\n";
        for(int i = 8; i < 16; i++)
        {
            if (GET_BIT(status, i))
            {
                statusList << this->mycmu->BmuBalErrStr_a[i - 8];
            }
        }
    }
    else if(this->mycmu->is_pVer_a_fan_pal())
    {
        for(int i = 0; i < 8; i++)
        {
            if (GET_BIT(status, i))
            {
                statusList << this->mycmu->BmuStateStr_m[i];
            }
            else
            {
                statusList << this->mycmu->BmuStateStr_m[i + 8];
            }
        }
        statusList << "\n";
        for(int i = 8; i < 16; i++)
        {
            if (GET_BIT(status, i))
            {
                statusList << this->mycmu->BmuBalErrStr_m[i - 8];
            }
        }
    }
    else if(this->mycmu->is_pVer_a_liq_mos())
    {
        for(int i = 0; i < 8; i++)
        {
            if (GET_BIT(status, i))
            {
                statusList << this->mycmu->BmuStateStr_w[i];
            }
            else
            {
                statusList << this->mycmu->BmuStateStr_w[i + 8];
            }
        }
        statusList << "\n";
        for(int i = 8; i < 16; i++)
        {
            if (GET_BIT(status, i))
            {
                statusList << this->mycmu->BmuBalErrStr_w[i - 8];
            }
        }
    }
    else
    {
        if (GET_BIT(status, 0)) statusList << QObject::tr("拨码异常");
        if (GET_BIT(status, 1)) statusList << QObject::tr("拨码锁定");
        statusList << (GET_BIT(status, 2) ? QObject::tr("干结点开路") : QObject::tr("干结点闭合"));
        statusList << (GET_BIT(status, 3) ? QObject::tr("风机开") : QObject::tr("风机关"));
        if (!GET_BIT(status, 4)) statusList << QObject::tr("辅源异常");
        if (GET_BIT(status, 5)) statusList << QObject::RESERVED_TEXT_RES;
        if (GET_BIT(status, 6)) statusList << QObject::RESERVED_TEXT_RES;
        if (GET_BIT(status, 7)) statusList << QObject::RESERVED_TEXT_RES;
        if (GET_BIT(status, 8)) statusList << QObject::tr("1.25V错误");
        if (GET_BIT(status, 9)) statusList << QObject::tr("均衡母线错误");
        if (GET_BIT(status, 10)) statusList << QObject::tr("均衡电流异常");
        if (GET_BIT(status, 11)) statusList << QObject::tr("24V母线异常");
        if (GET_BIT(status, 12)) statusList << QObject::tr("电芯电压异常");
        if (GET_BIT(status, 13)) statusList << QObject::tr("均衡参数错误");
        if (GET_BIT(status, 14)) statusList << QObject::tr("Mos异常");
        if (GET_BIT(status, 15)) statusList << QObject::tr("副边电压异常");
    }
    return statusList.join("|");
}
void BMSView::flushSoe(const ST_SOE& soe) {
    if (soe.list_soe.count()) {
        ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        if(this->mycmu->is_pVer_a_fan_pal()){
            m_model.setData(soe.list_soe, db_manager::SOE_BMS3);
        }else if(this->mycmu->is_exVer_3levels_alarm()){
            m_model.setData(soe.list_soe, db_manager::SOE_BMS4);
        }else{
            m_model.setData(soe.list_soe, db_manager::SOE_BMS2);
        }

        ui->labelSOE->setText(QString("New:%1,Total:%2").arg(soe.new_soe_count).arg(soe.soe_count));
        ui->ViewSOE->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}
void BMSView::flushBmuVolt(){
    // 一定要固定宽度，否则刷新很慢
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableBMU->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    QTableWidgetItem* item;
    int colOffset = 0;
    for (int i = 0; i < config.bmu_num; i++) {

        colOffset = 0;

        if(db_manager::Instance()->userLevel() == db_manager::LEVEL_GUEST)
        {
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
            }
            item->setFont(font);

            item->setToolTip(tr("删除线表示断线"));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableBMU->setItem(i, colOffset, item);
            colOffset++;
        }

        // 电芯电压
        for (int j = 0; j < config.vol_num; j++) {
            item = new QTableWidgetItem();
            double val = this->mycmu->bmu_data[i].Ucell[j] / 10000.0;
            if (this->mycmu->is_pVer_a_fan_pal()) {
                item->setText(QString("%1 [%2]").arg(val,5,'f', 3,'0').arg(mycmu->bmu_data[i].BalIdc[j] / 1000.0));
            }else if(this->mycmu->is_pVer_a_liq_mos()){
                QString str = "";
                if((mycmu->bmu_data[i].U64BalErr>>j)&0x01){
                    str = "[闭锁]";
                }
                item->setText(QString("%1 %2").arg(val,5,'f', 3,'0').arg(str));
            }else {
                item->setText(QString("%1").arg(val, 0, 'g', 5));
            }
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            QFont font = item->font();
            uint64_t breakLineTemp = 0;
            if (this->mycmu->is_pVer_a_liq_mos()){
                breakLineTemp = mycmu->bmu_data[i].U64break;
                //qWarning()<<"mycmu->bmu_data["<<i<<"].U64break:"<<mycmu->bmu_data[i].U64break;
            }else{
                breakLineTemp = mycmu->bmu_data[i].Ubreak;
            }
            if (GET_BIT(breakLineTemp, j)) {

                font.setStrikeOut(true);
            } else {

                font.setStrikeOut(false);
            }
            // pack最大标红
            if (mycmu->bmu_data[i].MaxUcellId == j) {
                // 簇最大标粗
                if (mycmu->bms_data.MaxUbmuId == i) {
                    item->setTextColor(QColor(Qt::red));
                    //font.setBold(true);
                }
            }
            if (mycmu->bmu_data[i].MinUcellId == j) {
                if (mycmu->bms_data.MinUbmuId == i) {
                    item->setTextColor(QColor(Qt::darkGreen));
                    //font.setItalic(true);
                }
            }

            if(this->mycmu->is_pVer_a_liq_mos()){
                if((mycmu->bmu_data[i].U64BalErr>>j)&0x01){
                    item->setTextColor(QColor(Qt::red));
                }
            }
            //
            item->setFont(font);
            item->setToolTip(tr("Strikethrough indicates disconnection"));
            ui->tableBMU->setItem(i, j + colOffset, item);
        }
    }

    // 数据刷新完毕后自适应列宽
    ui->tableBMU->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void BMSView::flushBmuTemp(){
    // 一定要固定宽度，否则刷新很慢
    ui->tableTemp->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableTemp->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    for (int i = 0; i < config.bmu_num; i++) {
        for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
            QTableWidgetItem* item = new QTableWidgetItem();
            double val = this->mycmu->bmu_data[i].Tcell[j] / 10.0;
            item->setText(QString("%1").arg(val, 0, 'g', 5));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            QFont font = item->font();
            uint64_t TbreakLineTemp = 0;
            if (this->mycmu->is_pVer_a_liq_mos()){
                TbreakLineTemp = mycmu->bmu_data[i].T64break;
            }else{
                TbreakLineTemp = mycmu->bmu_data[i].Tbreak;
            }
            if (GET_BIT(TbreakLineTemp, j)) {
                font.setStrikeOut(true);
            } else {
                font.setStrikeOut(false);
            }

            // pack最大标红
            if (mycmu->bmu_data[i].MaxTcellId == j) {
                // 簇最大标粗
                if (mycmu->bms_data.MaxTbmuId == i) {
                    item->setTextColor(QColor(Qt::red));
                    //font.setBold(true);
                }
            }
            if (mycmu->bmu_data[i].MinTcellId == j) {
                // 簇最小标斜体
                if (mycmu->bms_data.MinTbmuId == i) {
                    item->setTextColor(QColor(Qt::darkGreen));
                    font.setItalic(true);
                }
            }
            item->setFont(font);
            item->setToolTip(tr("Strikethrough indicates disconnection"));
            ui->tableTemp->setItem(i, j, item);
        }
    }

    // 数据刷新完毕后自适应列宽
    ui->tableTemp->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void BMSView::flushBmuVer(){
    // 一定要固定宽度，否则刷新很慢
    ui->tableVer->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableVer->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    int cloumn_offset = 0;
    QTableWidgetItem* item;


    //访客权限的特殊处理
    if(db_manager::Instance()->userLevel() == db_manager::LEVEL_GUEST)
    {
        QList<verLabel_T> list;
        verLabel_T verLabel;
        for (int i = 0; i < config.bmu_num; i++)
        {
            verLabel.ver = myHelper::IntegerToHexString(mycmu->bmu_data[i].Version);
            verLabel.onLine = this->bmu_comm >> i & 0x01;
            list.append(verLabel);
        }
        ui->sysStatWd->setVerList(list);
    }

    for (int i = 0; i < config.bmu_num; i++) {
        cloumn_offset = 0;
        // 版本号
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
            //font.setBold(true);
        }
        item->setFont(font);
        //item->setToolTip(tr("Strikethrough indicates disconnection"));
        item->setToolTip(tr("删除线表示断线"));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableVer->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].RunStat, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setToolTip(getBmuInfo(mycmu->bmu_data[i].RunStat));
        ui->tableVer->setItem(i, cloumn_offset++, item);

        item = new QTableWidgetItem();
        item->setText(QString("0x%1").arg(mycmu->bmu_data[i].ErrStat, 4, 16, QLatin1Char('0')));
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        item->setToolTip(getBmuErrInfo(mycmu->bmu_data[i].ErrStat));
        ui->tableVer->setItem(i, cloumn_offset++, item);

        if (this->mycmu->is_pVer_active())
        {

            if (this->mycmu->is_cpVer_with_fan_rate())
            {
                item = new QTableWidgetItem();
                item->setText(QString("%1").arg(mycmu->bmu_data[i].FanSpeed));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
            }
            else if(this->mycmu->is_pVer_a_liq_mos())
            {

            }
            else
            {
                item = new QTableWidgetItem();
                QString fanStatus = GET_BIT(mycmu->bmu_data[i].RunStat, 3) ? tr("ON") : tr("OFF");
                item->setText(fanStatus);
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
            }


            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(mycmu->bmu_data[i].BalU24 / 1000.0));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableVer->setItem(i, cloumn_offset++, item);


            if (this->mycmu->is_pVer_a_fan_pal())
            {
                item = new QTableWidgetItem();
                item->setText(QString("%1").arg((float)mycmu->bmu_data[i].BalI48 / 1000));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
            }
            else if (this->mycmu->is_pVer_a_fan_pal())
            {
                item = new QTableWidgetItem();
                double max_bal_current = 0;
                for (int k = 0; k < config.vol_num; k++) {
                    max_bal_current += mycmu->bmu_data[i].BalIdc[k] / 1000.0;
                }
                item->setText(QString("%1").arg(max_bal_current));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
            }
            else if(this->mycmu->is_pVer_a_liq_mos())
            {
                QStringList strl = mycmu->GetBalanceValue(mycmu->bmu_data[i].U64BalStat,
                                                          mycmu->bmu_data[i].BalIdc).split("|");
//                qDebug() << strl;
                if(strl.count()>4){
                    for (int j = 0; j < 4; ++j) {
                        item = new QTableWidgetItem();
                        item->setText("");
                        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                        ui->tableVer->setItem(i, cloumn_offset++, item);
                    }
                }else{
                    for (int j = 0; j < 4; ++j) {
                        item = new QTableWidgetItem();
                        item->setText(strl[j]);
                        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                        ui->tableVer->setItem(i, cloumn_offset++, item);
                    }
                }
            }
            else
            {
                item = new QTableWidgetItem();
                item->setText(QString("%1").arg(mycmu->bmu_data[i].BalIdc[0] / 1000.0));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
            }

            if(this->mycmu->is_pVer_a_liq_mos())
            {
                item = new QTableWidgetItem();
                item->setText(mycmu->GetBalanceValue(mycmu->bmu_data[i].BalMode,mycmu->bmu_data[i].BalCur));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
//                qDebug() << mycmu->bmu_data[i].BalMode << mycmu->bmu_data[i].BalCur;
            }
            else
            {
                item = new QTableWidgetItem();
                item->setText(mycmu->GetBitStatus(mycmu->bmu_data[i].BalErr));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);

                item = new QTableWidgetItem();
                item->setText(mycmu->GetBitStatus(mycmu->bmu_data[i].BalStat));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);

                item = new QTableWidgetItem();
                item->setText(mycmu->GetBalanceValue(mycmu->bmu_data[i].BalMode));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableVer->setItem(i, cloumn_offset++, item);
            }

            // CAN通信错误计数
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(this->mycmu->bmu_data[i].CanErr));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableVer->setItem(i, cloumn_offset++, item);
        }
        else if (mycmu->is_cpVer_Higher_than(CMU_P_V0_0_02))
        {
            // CAN通信错误计数
            item = new QTableWidgetItem();
            item->setText(QString("%1").arg(this->mycmu->bmu_data[i].CanErr));
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            ui->tableVer->setItem(i, cloumn_offset++, item);
        }
    }

    // 数据刷新完毕后自适应列宽
    ui->tableVer->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void BMSView::flushBmuEx(){
    // 一定要固定宽度，否则刷新很慢
    ui->tableExtView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableExtView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    if (ui->tabBalance->isEnabled()) {
        QTableWidgetItem* item;
        int offset = 0;
        ui->tableExtView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->tableExtView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        for (int i = 0; i < config.bmu_num; i++) {
            if(this->mycmu->is_pVer_a_fan_pal()){
                offset = 0;

                uint16_t hw;
                item = new QTableWidgetItem();
                hw = this->mycmu->bmu_data[i].HVersion;
                item->setText(QString("%1.%2").arg(hw>>8,2,16,QChar('0')).arg((uint8_t)hw,2,16,QChar('0')));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);

                uint32_t bootversion;
                item = new QTableWidgetItem();
                bootversion = this->mycmu->bmu_data[i].BMUBootVersion;
                item->setText(QString("%1.%2.%3.%4").arg((uint8_t)(bootversion>>24),2,16,QChar('0'))
                              .arg((uint8_t)(bootversion>>16),2,16,QChar('0'))
                              .arg((uint8_t)(bootversion>>8),2,16,QChar('0'))
                              .arg((uint8_t)(bootversion>>0),2,16,QChar('0')));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);

                uint16_t sn;
                item = new QTableWidgetItem();
                sn = this->mycmu->bmu_data[i].BMUSN;
                QString str = "";
                if(sn == 0x0069){
                    str = tr("金升阳");
                }else if(sn == 0x0060){
                    str = tr("爱浦");
                }else if(sn == 0x006B){
                    str = tr("源特");
                }else{
                    str = tr("未知");
                }

                item->setText(QString("%1.%2[%3]").arg((uint8_t)(sn>>8),0,10)
                              .arg((uint8_t)sn,0,10).arg(str));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);

                double T;
                item = new QTableWidgetItem();
                T = this->mycmu->bmu_data[i].ModT1/10.0;
                item->setText(QString("%1").arg(T,0,'g',5));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);

                item = new QTableWidgetItem();
                T = this->mycmu->bmu_data[i].ModT2/10.0;
                item->setText(QString("%1").arg(T,0,'g',5));
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                ui->tableExtView->setItem(i, offset++, item);

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
            }else{
                offset = 0;

                if(this->mycmu->is_cpVer_a_fan_mos_with_boot_ver()|| this->mycmu->is_pVer_a_liq_mos()){
                    offset = 0;
                    uint32_t bootversion;
                    item = new QTableWidgetItem();
                    bootversion = this->mycmu->bmu_data[i].BMUBootVersion;
                    item->setText(QString("%1.%2.%3.%4").arg((uint8_t)(bootversion>>24),2,16,QChar('0'))
                                  .arg((uint8_t)(bootversion>>16),2,16,QChar('0'))
                                  .arg((uint8_t)(bootversion>>8),2,16,QChar('0'))
                                  .arg((uint8_t)(bootversion>>0),2,16,QChar('0')));
                    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                    ui->tableExtView->setItem(i, offset++, item);
                }

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
        }
        ui->tableExtView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableExtView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }

    // 数据刷新完毕后自适应列宽
    ui->tableExtView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void BMSView::flushBmu() {
    if (!mycmu) return;
    if (config.bmu_num > ui->tableBMU->rowCount()) return;



    flushBmuVolt();
    flushBmuTemp();
    flushBmuVer();
    flushBmuEx();

}
struct mb_cmd {
    uint16_t type;
    uint16_t addr;
    uint16_t value;
};
static map<QString, mb_cmd> btnMap = {
    //{"btnBMULock", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_BMU_LOCK}},
    //{"btnBMUUnlock", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_BMU_UNLOCK}},
    {"btnClearEng", {CTRL_AO_ADDR, ADDR_CLEAR_ENG, MB_CLEAR_ENG}},
    {"btnUploadTrig", {CTRL_AO_ADDR, ADDR_CLEAR_ENG, MB_UPLOAD_Trig}},
    {"btnIFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_IFull}},
    {"btnIBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_IBase}},
    {"btnIZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_IZero}},
    //{"btnIleakFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_LFull}},
    {"btnIleakBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_LBase}},
    //{"btnIleakZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_LZero}},
    //{"btnRFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_TFull}},
    {"btnRBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_TBase}},
    //{"btnRZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_TZero}},
    //{"btnUFullAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_VFull}},
    {"btnUBaseAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_VBase}},
    //{"btnUZeroAdj", {CTRL_SEC_AO, ADDR_ADJ, MB_Adj_VZero}},
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
    {"btnClrSysLock", {CTRL_AO_ADDR, ADDR_CLEAR_SYSLOCK, MB_CLR_SYSLOCK}},
    {"btnFanON", {CTRL_AO_ADDR, 0xFF0B, 0xa5fe}},
    {"btnFanOFF", {CTRL_AO_ADDR, 0xFF0B, 0x5aff}},
    {"AutoFindAddr", {CTRL_AO_ADDR, ADDR_CTRL_FINDADDR, MB_CTRL_ON}},
    {"btnHeRongON", {CTRL_AO_ADDR, ADDR_CTRL_HR, MB_CTRL_ON}},
    {"btnHeRongOFF", {CTRL_AO_ADDR, ADDR_CTRL_HR, MB_CTRL_OFF}},
    {"btnAcON", {CTRL_AO_ADDR, ADDR_CTRL_AC, MB_CTRL_ON}},
    {"btnAcOFF", {CTRL_AO_ADDR, ADDR_CTRL_AC, MB_CTRL_OFF}},
    {"btnResON", {CTRL_AO_ADDR, ADDR_CTRL_RES, MB_CTRL_ON}},
    {"btnResOFF", {CTRL_AO_ADDR, ADDR_CTRL_RES, MB_CTRL_OFF}},
    {"btnTimeAdj", {CERT_CMD_TIME_ADJ, 0, 0}},
    //{"btnResetDef", {CTRL_AO_ADDR, ADDR_RESET_FACTORY, MB_FACTORY}},
    {"btnCan485SelfDetectOn", {CTRL_AO_ADDR, ADDR_CTRL_COMM_SELF_DETECTE, MB_CTRL_ON}}};


void BMSView::AOCtrlEmit(uint16_t v1, uint16_t v2, QString info)
{
    if (myHelper::ShowMessageBoxQuesion(info) == QDialog::Accepted) {
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_AO_ADDR;
        uint16_t value[2];
        value[0] = v1;
        value[1] = v2;
        MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
    }
}
void BMSView::AOCtrlEmit(uint16_t v1, uint16_t v2)
{
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_AO_ADDR;
    uint16_t value[2];
    value[0] = v1;
    value[1] = v2;
    MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
    if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
}

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

            qInfo() << "btnInfo>>>>>>>>>>>>>>" << "btn Accepted" << name;
            if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
        }
    }else if (name == "btnRZeroAdj") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        val[0] = ADDR_ADJ;
        val[1] = MB_Adj_TZero;
        bool lbok;
        QString value = myHelper::showInputBox(tr("Ain电流传感器低点校准(-4V< X <0V)"), lbok,"-3.8");
        if (lbok) {
            double dval = value.toDouble(&lbok)+4;
            if (lbok) {
                if(0<dval&&dval<4){
                    val[2] = (dval-4)*375+1500;
                    qDebug() << "RZeroAdj:" << val[2];
                    MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                }else{
                    myHelper::ShowMessageBoxError(tr("invalid value:%1 must >-4 && < 0!").arg(value));
                }

            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

    } else if (name == "btnRFullAdj") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        val[0] = ADDR_ADJ;
        val[1] = MB_Adj_TFull;
        bool lbok;
        QString value = myHelper::showInputBox(tr("Ain电流传感器高点校准(0V< X <4V)"), lbok,"3.8");
        if (lbok) {
            double dval = value.toDouble(&lbok);
            if (lbok) {
                if(0<dval&&dval<4){
                    val[2] = dval*375+1500;
                    qDebug() << "RFullAdj:" << val[2];
                    MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                }else{
                    myHelper::ShowMessageBoxError(tr("invalid value:%1 must >0 && <4!").arg(value));
                }

            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

    }else if (name == "btnUZeroAdj") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        val[0] = ADDR_ADJ;
        val[1] = MB_Adj_VZero;
        bool lbok;
        QString value = myHelper::showInputBox(tr("预留电流传感器低点校准(-4V< X <0V)"), lbok,"-3.8");
        if (lbok) {
            double dval = value.toDouble(&lbok)+4;
            if (lbok) {
                if(0<dval&&dval<4){
                    val[2] = (dval-4)*375+1500;
                    qDebug() << "btnUZeroAdj:" << val[2];
                    MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                }else{
                    myHelper::ShowMessageBoxError(tr("invalid value:%1 must >-4 && < 0!").arg(value));
                }

            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

    } else if (name == "btnUFullAdj") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        val[0] = ADDR_ADJ;
        val[1] = MB_Adj_VFull;
        bool lbok;
        QString value = myHelper::showInputBox(tr("预留电流传感器高点校准(0V< X <4V)"), lbok,"3.8");
        if (lbok) {
            double dval = value.toDouble(&lbok);
            if (lbok) {
                if(0<dval&&dval<4){
                    val[2] = dval*375+1500;
                    qDebug() << "btnUFullAdj:" << val[2];
                    MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                }else{
                    myHelper::ShowMessageBoxError(tr("invalid value:%1 must >0 && <4!").arg(value));
                }

            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

    }else if (name == "btnIleakZeroAdj") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        val[0] = ADDR_ADJ;
        val[1] = MB_Adj_LZero;
        bool lbok;
        QString value = myHelper::showInputBox(tr("漏电流低点校准(-5V< X <0V)"), lbok,"-4.8");
        if (lbok) {
            double dval = value.toDouble(&lbok)+5;
            if (lbok) {
                if(0<dval&&dval<5){
                    val[2] = (dval-5)*300+1500;
                    qDebug() << "btnIleakZeroAdj:" << val[2];
                    MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                }else{
                    myHelper::ShowMessageBoxError(tr("invalid value:%1 must >-5 && < 0!").arg(value));
                }

            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

    } else if (name == "btnIleakFullAdj") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        val[0] = ADDR_ADJ;
        val[1] = MB_Adj_LFull;
        bool lbok;
        QString value = myHelper::showInputBox(tr("漏电流高点校准(0V< X <5V)"), lbok,"4.8");
        if (lbok) {
            double dval = value.toDouble(&lbok);
            if (lbok) {
                if(0<dval&&dval<5){
                    val[2] = dval*300+1500;
                    qDebug() << "btnIleakFullAdj:" << val[2];
                    MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                }else{
                    myHelper::ShowMessageBoxError(tr("invalid value:%1 must >0 && <5!").arg(value));
                }

            } else {
                myHelper::ShowMessageBoxError(tr("invalid value:%1!").arg(value));
            }
        }
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

    }else if (name == "btnRUAdj") {
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
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

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
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);

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
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
    } else if (name == "btnBalance") {
        StartBalanceForm();
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
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
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
            AOCtrlEmit(65287, mode);
        }
    } else if (name == "btnRClrErr") {
        AOCtrlEmit(65288, 0xAA55, ui->btnRClrErr->text() + "?");
    }  else if (name == "btnResetInsVCali") {
        AOCtrlEmit(65288, 0xBB66, ui->btnResetInsVCali->text() + "?");
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
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
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
        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
    } else if (name == "btnAdjSOC") {
        AOCtrlEmit(65525, 0x1EA5, tr("是否校准SOC？"));
    } else if (name == "btnRebootBMUs") {
        rebootbmus->show();
    } else if (name == "btnBalClrErr") {
        AOCtrlEmit(65289, 0xAA55, ui->btnBalClrErr->text() + "?");
    } else if (name == "btnBmuResetCaliCfg"){
        AOCtrlEmit(0xFF0A, 0xAA55, ui->btnBmuResetCaliCfg->text() + "?");
    } else if (name == "btnBmuResetRunCfg"){
        AOCtrlEmit(0xFF0A, 0xBB66, ui->btnBmuResetRunCfg->text() + "?");
    } else if (name == "btnBmuResetParaCfg"){
        AOCtrlEmit(0xFF0A, 0xCC77, ui->btnBmuResetParaCfg->text() + "?");
    } else if (name == "btnBmuResetBalCfg"){
        AOCtrlEmit(0xFF0A, 0xDD88, ui->btnBmuResetBalCfg->text() + "?");
    } else if (name == "btnBmuResetDef"){
        AOCtrlEmit(0xFF0A, 0x1D32, ui->btnBmuResetDef->text() + "?");
    } else if (name == "btnBMUUnlock"){
        AOCtrlEmit(0xFFF1, 0x55AA, ui->btnBMUUnlock->text() + "?");
    } else if (name == "btnBMULock"){
        AOCtrlEmit(0xFFF1, 0xAA55, ui->btnBMULock->text() + "?");
    } else if (name == "btnCmuResetCaliCfg"){
        AOCtrlEmit(0xFFF1, 0xA555, ui->btnCmuResetCaliCfg->text() + "?");
    } else if (name == "btnCmuResetRunCfg"){
        AOCtrlEmit(0xFFF1, 0xB666, ui->btnCmuResetRunCfg->text() + "?");
    } else if (name == "btnCmuResetParaCfg"){
        AOCtrlEmit(0xFFF1, 0xC777, ui->btnCmuResetParaCfg->text() + "?");
    } else if (name == "btnCmuResetBalCfg"){
        AOCtrlEmit(0xFFF1, 0xD888, ui->btnCmuResetBalCfg->text() + "?");
    } else if (name == "btnCmuResetDef"){
        AOCtrlEmit(0xFFF1, 0x1D32, ui->btnCmuResetDef->text() + "?");
    } else if(name.startsWith("btnDebugRelayCtrl")){
        bool ctrl = false;
        QString str = "";
        QString strBit = name;
        strBit.remove("btnDebugRelayCtrl").remove("On").remove("Off").remove("ON").remove("OFF");
        uint16_t bit = strBit.toUInt();

        if(bit > 15){
            return;
        }

        if(name.endsWith("On")||name.endsWith("ON")){
            ctrl = true;
            str = tr("测试继电器-bit%1-开启确认").arg(bit);
        }else{
            str = tr("测试继电器-bit%1-关闭确认").arg(bit);
        }
        if (myHelper::ShowMessageBoxQuesion(str) == QDialog::Accepted) {

            qInfo() << "btnInfo>>>>>>>>>>>>>>" << "btnIOCtrl"+strBit << bit << ctrl;
            uint16_t value[2] = {0};
            value[0] = bit+1;
            value[1] = ctrl;

            TMsgData MsgCmd;
            MsgCmd.msg_type = CTRL_DO;
            MsgCmd.data.append((char*)&value, 2 * sizeof(uint16_t));
            emit send_msg(MsgCmd);
        }
    }else
        qDebug() << "don`t define :" << name ;
}
void BMSView::stateChanged() {
    QCheckBox* b = (QCheckBox*)sender();
    QList<QCheckBox*> CheckBoxList;
    uint16_t value[2] = {0};
    TMsgData MsgCmd;
    NodeReg node;
    MsgCmd.msg_type = CTRL_AO_ADDR;


    if(b->objectName().startsWith("bFuncEx")){
        CheckBoxList << ui->bFuncEx00 << ui->bFuncEx01 << ui->bFuncEx02 << ui->bFuncEx03 << ui->bFuncEx04 << ui->bFuncEx05 << ui->bFuncEx06
                     << ui->bFuncEx07 << ui->bFuncEx08 << ui->bFuncEx09 << ui->bFuncEx10 << ui->bFuncEx11 << ui->bFuncEx12 << ui->bFuncEx13
                     << ui->bFuncEx14 << ui->bFuncEx15;
        node = mycmu->GetNodeAddr("FuncMask2");

    }else if(b->objectName().startsWith("bFunc")){
        CheckBoxList << ui->bFunc00 << ui->bFunc01 << ui->bFunc02 << ui->bFunc03 << ui->bFunc04 << ui->bFunc05 << ui->bFunc06
                     << ui->bFunc07 << ui->bFunc08 << ui->bFunc09 << ui->bFunc10 << ui->bFunc11 << ui->bFunc12 << ui->bFunc13
                     << ui->bFunc14 << ui->bFunc15;
        node = mycmu->GetNodeAddr("FuncMask");

    }else{
        return;
    }


    foreach (QCheckBox* cb, CheckBoxList) {
        value[1] |= (cb->isChecked() << CheckBoxList.indexOf(cb));
    }

    if (myHelper::ShowMessageBoxQuesion(
                QString(tr("确定%2\"%1\"吗").arg(b->text()).arg(b->isChecked() > 0 ? tr("开启") : tr("关闭")))) !=
            QDialog::Accepted)
        return;

    if (node.reg_type > 0) {
        value[0] = node.reg_addr;
        MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
        emit send_msg(MsgCmd);
    }
}
void BMSView::checkChanged() {
    QCheckBox* b = (QCheckBox*)sender();
    QList<QCheckBox*> RadioList;
    uint16_t value[2] = {0};
//    RadioList << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3 << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
//              << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11 << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;
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
    emit send_msg(MsgCmd);
}
void BMSView::btn_contrl() {
    TMsgData MsgCmd;
    QPushButton* b = (QPushButton*)sender();
    QString name = b->objectName();
    if (name == "btnReadSOE") {
        MsgCmd.msg_type = CERT_CMD_READ_SOE;
        MsgCmd.data.clear();
        emit send_msg(MsgCmd);
        ui->labelSOE->setText(tr("读取中...请稍侯..."));
    } else if (name == "btnClearSOE") {
        if (myHelper::ShowMessageBoxQuesion("Sure to clear All SOE ?") == QDialog::Accepted) {
            MsgCmd.msg_type = CTRL_AO_ADDR;
            uint16_t val[2] = {0xFFFB, 0xBB66};
            MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
            emit send_msg(MsgCmd);
            MsgCmd.data.clear();
        }
    } else
        qDebug()<< "don`t define :" << name;
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
    emit send_msg(MsgCmd);
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
    emit send_msg(MsgCmd);
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
    QList<InputBox*> dspboxs = ui->tabSet->findChildren<InputBox*>();
    foreach (InputBox* dspbox, dspboxs) {
        if (!dspbox->isHidden()) {
            QDomElement item1 = document.createElement("item");
            item1.setAttribute("name", dspbox->objectName());
            item1.setAttribute("name_cn", dspbox->prefix().trimmed());
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
    QDomElement root = doc.documentElement();  // 返回根节点
    QDomNode node = root.firstChild();         // 获得第一个子节点
    while (!node.isNull())                     // 如果节点不空
    {
        if (node.isElement())  // 如果节点是元素
        {
            QDomElement e = node.toElement();  // 转换为元素，注意元素和节点是两个数据结构，其实差不多
            InputBox* dspbox = ui->tabSet->findChild<InputBox*>(e.attribute("name"));
            if (dspbox != nullptr) {
                if (!dspbox->isHidden()) {
                    double value = e.attribute("value").toDouble();
                    if (value != dspbox->value()) setValue(dspbox->objectName(), value);
                }
            }
        }
        node = node.nextSibling();  // 下一个兄弟节点,nextSiblingElement()是下一个兄弟元素，都差不多
    }
    doc.clear();
    qDebug() << "Paramet File load ok!";
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
// 屏蔽本控件传递事件到父控件
bool BMSView::eventFilter(QObject* obj, QEvent* event) {
    Q_UNUSED(obj);
    Q_UNUSED(event);
    if (obj == ui->tableBMU) {
        qDebug() << "ignore event for parent:" << obj << event;
    }

    return true;  // QWidget::eventFilter(obj, event);
}
void BMSView::on_cbProtocol_currentIndexChanged(const QString& arg1) {
    qDebug() << "Protocol_Changed:" << arg1;
    settings->setValue("global/protocol", arg1);
    TMsgData MsgCmd;
    MsgCmd.msg_type = CTRL_SET_PRO;
    MsgCmd.data.setNum(ui->cbProtocol->currentData().toUInt());
    emit send_msg(MsgCmd);
    MsgCmd.data.clear();

}

void BMSView::on_cbProtocol_activated(int index)
{
//    qDebug() << "Protocol_Changed:" << index;
//    settings->setValue("global/protocol", index);
//    TMsgData MsgCmd;
//    MsgCmd.msg_type = CTRL_SET_PRO;
//    MsgCmd.data.setNum(ui->cbProtocol->currentData().toUInt());
//    emit send_msg(MsgCmd);
//    MsgCmd.data.clear();
//    ui->sysStatWd->setProtocol(BMS_PROTOCOL(ui->cbProtocol->currentData().toUInt()));
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

    foreach(InputBox* ib, ui->groupBoxCMU->findChildren<InputBox*>())
    {
        ib->setFontAndSize(8);
    }

    ui->btnVer->setMenu(update_menu);
    {
#if UI_IP_SEL == UI_IP_LE
        ui->refreashIp->setVisible(false);
        ui->cbConnectIP->setVisible(false);
        connect(ui->connectIP, &QLineEdit::editingFinished, this, &BMSView::IpChange, Qt::UniqueConnection);
#endif
#if UI_IP_SEL == UI_IP_CB
        ui->connectIP->setVisible(false);
        connect(ui->cbConnectIP, &QComboBox::editTextChanged, this, &BMSView::IpChange, Qt::UniqueConnection);
#endif
        connect(ui->tbtnConnect, SIGNAL(clicked(bool)), this, SLOT(btnClick()));
        //
        QList<InputBox*> dspboxs = ui->tabSet->findChildren<InputBox*>();
        foreach (InputBox* dspbox, dspboxs) {
            connect(dspbox, static_cast<void (InputBox::*)(double)>(&InputBox::valueChanged), this,
                    &BMSView::valueChange, Qt::UniqueConnection);
        }
        connect(ui->BalnceMask, static_cast<void (QWidget::*)(const QPoint& pos)>(&QWidget::customContextMenuRequested),
                this,
                [=](const QPoint& pos) {  // Handle global position
            QPoint globalPos = ui->BalnceMask->mapToGlobal(pos);
            // Create menu and insert some actions
            QMenu myMenu;
            myMenu.addAction(tr("修改"), this, [=]() {
                StartBalanceForm();
            });
            // Show context menu at handling position
            myMenu.exec(globalPos);
        });
        connect(ui->BalnceStart,
                static_cast<void (QWidget::*)(const QPoint& pos)>(&QWidget::customContextMenuRequested), this,
                [=](const QPoint& pos) {  // Handle global position
            QPoint globalPos = ui->BalnceStart->mapToGlobal(pos);
            // Create menu and insert some actions
            QMenu myMenu;
            myMenu.addAction(tr("修改均衡配置"), this, [=]() {
                int mode = ui->BalnceStart->value();
                if (configBalance == nullptr) {
                    configBalance = new frmbalanceConfig();
                }
                configBalance->protocal_ver = this->mycmu->GetProtocalVer();
                configBalance->setValue(mode);
                if (configBalance->exec() == QDialog::Accepted) {
                    TMsgData MsgCmd;
                    MsgCmd.msg_type = CTRL_AO_ADDR;
                    uint16_t value[2] = {5409, 0};
                    value[1] = configBalance->getValue();
                    MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
                    if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
                }
            });
            // Show context menu at handling position
            myMenu.exec(globalPos);
        });

        connect(ui->AllOnOffPower,
                static_cast<void (QWidget::*)(const QPoint& pos)>(&QWidget::customContextMenuRequested), this,
                [=](const QPoint& pos) {  // Handle global position
            QPoint globalPos = ui->BMUPower->mapToGlobal(pos);
            // Create menu and insert some actions
            QMenu myMenu;
            myMenu.addAction(tr("修改"), this, [=]() {
                SwitchPowerConfig spc(NULL, PCS_POWER_SET);
                QVector<uint16_t> data;
                uint16_t read;

                if(pcsPowerMapDataCache.contains("SwitchONACPower")){
                    uint16_t read = pcsPowerMapDataCache.value("SwitchONACPower");
                    data<< 5450 << read;
                }
                if(pcsPowerMapDataCache.contains("SwitchPower")){
                    uint16_t read = pcsPowerMapDataCache.value("SwitchPower");
                    data<< 5451 << read;
                }
                if(pcsPowerMapDataCache.contains("SwitchOFFPower")){
                    uint16_t read = pcsPowerMapDataCache.value("SwitchOFFPower");
                    data<< 5452 << read;
                }
                if(pcsPowerMapDataCache.contains("NoAcPCSOnPower")){
                    uint16_t read = pcsPowerMapDataCache.value("NoAcPCSOnPower");
                    data<< 5454 << read;
                }
                if(pcsPowerMapDataCache.contains("NoAcPCSOffPower")){
                    uint16_t read = pcsPowerMapDataCache.value("NoAcPCSOffPower");
                    data<< 5455 << read;
                }

                spc.setValue(data);
                if (spc.exec() == QDialog::Accepted) {
                    data = spc.getValuePcs();
                    qDebug()<<"<<<"<<data;

                    for(int i = 0; i < data.count()/2; i++){
                        uint16_t value[2];
                        value[0] = data[i*2];
                        value[1] = data[i*2+1];
                        TMsgData MsgCmd;

                        MsgCmd.msg_type = CTRL_AO_ADDR;
                        MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
                        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
                    }
                }

            });
            // Show context menu at handling position
            myMenu.exec(globalPos);
        });

        connect(ui->BMUPower,
                static_cast<void (QWidget::*)(const QPoint& pos)>(&QWidget::customContextMenuRequested), this,
                [=](const QPoint& pos) {  // Handle global position
            QPoint globalPos = ui->BMUPower->mapToGlobal(pos);
            // Create menu and insert some actions
            QMenu myMenu;
            myMenu.addAction(tr("修改"), this, [=]() {
                SwitchPowerConfig spc(NULL, BMU_POWER_SET);
                spc.setMessage(tr("BMU功耗配置"),tr("BMU功耗"),tr("保留"));
                spc.setValue(ui->BMUPower->value());
                if (spc.exec() == QDialog::Accepted) {
                    TMsgData MsgCmd;
                    MsgCmd.msg_type = CTRL_AO_ADDR;
                    uint16_t value[2] = {5453, 0};
                    value[1] = spc.getValueBmu();
                    MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
                    if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
                }
            });
            // Show context menu at handling position
            myMenu.exec(globalPos);
        });

        QList<QPushButton*> btns = ui->tabCtrl->findChildren<QPushButton*>();
        foreach (QPushButton* btn, btns) {
            btn->setStyle(new QtPushButtonStyleProxy());
            connect(btn, &QPushButton::released, this, &BMSView::sendCommand, Qt::UniqueConnection);
        }
        QList<QCheckBox*> chkboxs = ui->G_FuncMask->findChildren<QCheckBox*>();
        foreach (QCheckBox* chkbox, chkboxs) {
            connect(chkbox, &QCheckBox::stateChanged, this, &BMSView::stateChanged, Qt::UniqueConnection);
        }

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
        ui->tableVer->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->tableVer,
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
    rebootMenu->addAction(tr("重启n次BMU"), this, &BMSView::sendCommand);
    rebootMenu->actions().constLast()->setObjectName("btnRebootBMUs");
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
    emit send_msg(MsgCmd);
    MsgCmd.data.clear();
    return;
}

void BMSView::on_checkBox_stateChanged(int arg1) {
    //qDebug() << QString("%1").arg(arg1);
    TMsgData MsgCmd;
    QCheckBox* cbox = (QCheckBox*)this->sender();
    if (cbox->isChecked()) {
        MsgCmd.msg_type = CTRL_DUMP;
        MsgCmd.data.clear();
    } else {
        MsgCmd.msg_type = CTRL_DUMP;
        MsgCmd.data.append("0");
    }
    emit send_msg(MsgCmd);
    MsgCmd.data.clear();
}
void BMSView::on_cb_ErrorLog_stateChanged(int arg1)
{
    //qDebug() << QString("%1").arg(arg1);
    TMsgData MsgCmd;
    QCheckBox* cbox = (QCheckBox*)this->sender();
    if (cbox->isChecked()) {
        MsgCmd.msg_type = CTRL_DUMPERRLOG;
        MsgCmd.data.clear();
    } else {
        MsgCmd.msg_type = CTRL_DUMPERRLOG;
        MsgCmd.data.append("0");
    }
    emit send_msg(MsgCmd);
    MsgCmd.data.clear();
}

void BMSView::btnClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->objectName();
    if (name == "connect" || name == "reconnect") {
        uint16_t port = ui->spinBoxPort->value();
        TMsgData MsgCmd;
        MsgCmd.msg_type = CONFIG_IP;

#if UI_IP_SEL == UI_IP_LE
        MsgCmd.data.append(ui->connectIP->text());
#endif
#if UI_IP_SEL == UI_IP_CB
        if (!myHelper::IsIP(ui->cbConnectIP->currentText())) {
            myHelper::ShowMessageBoxError(tr("invalid ip address!"));
            ui->cbConnectIP->setCurrentText(settings->value("global/target_ip", "192.168.1.120").toString());
            return;
        }
        MsgCmd.data.append(ui->cbConnectIP->currentText());
#endif
        emit send_msg(MsgCmd);
        MsgCmd.msg_type = CONFIG_PORT;
        MsgCmd.data.clear();
        MsgCmd.data.append((char*)&port, sizeof(port));
        emit send_msg(MsgCmd);
        MsgCmd.msg_type = CONFIG_INIT;
        MsgCmd.data.clear();
        emit send_msg(MsgCmd);
    }
}
void BMSView::IpChange() {
#if UI_IP_SEL == UI_IP_LE
    QLineEdit* pEdit = (QLineEdit*)sender();
    if (!pEdit->isModified()) return;
    pEdit->setModified(false);
    QString ip = pEdit->text();
    if (!myHelper::IsIP(ip)) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        pEdit->undo();
        return;
    }
#endif
#if UI_IP_SEL == UI_IP_CB
    QComboBox* pEdit = (QComboBox*)sender();
    QString ip = pEdit->currentText();
    if (false/*!myHelper::IsIP(ip)*/) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        pEdit->setCurrentText(settings->value("global/target_ip", "192.168.1.120").toString());
        return;
    }
#endif

    TMsgData MsgCmd;
    MsgCmd.msg_type = CONFIG_IP;
    MsgCmd.data.append(ip);
    emit send_msg(MsgCmd);
    settings->setValue("global/target_ip", ip);
}

bool BMSView::load_config() {
    settings = new QSettings("config.ini", QSettings::IniFormat);
    QString target_ip = settings->value("global/target_ip", "192.168.1.120").toString();
    int target_port = settings->value("global/target_port", 502).toUInt();

#if UI_IP_SEL == UI_IP_LE
        ui->connectIP->setText(target_ip);
#endif
#if UI_IP_SEL == UI_IP_CB
    ui->cbConnectIP->setCurrentText(target_ip);
#endif
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
        if((!this->mycmu->is_pVer_a_fan_mos()) && (!this->mycmu->is_pVer_a_fan_pal()) )
        {
            return;
        }

        QTableWidget* table = ui->tableVer;
        //        QPoint globalPos = table->mapToGlobal(pos);
        QModelIndex index = table->indexAt(pos);        
        //qDebug() << index.row();
        // Create menu and insert some actions
        QMenu* myMenu = new QMenu(table);
//        myMenu->addAction(tr("导出当前数据"), this, [=]() {
//            QString fileName = QFileDialog::getSaveFileName(
//                        this, tr("Save File"), tr("BMU数据") + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
//                        tr("csv File(*.csv)"));
//            if (fileName.isNull()) {
//                return;
//            }
//            exportExecl(table, fileName);
//        });
        myMenu->addAction(QString("%1:BMU%2").arg(tr("开启风扇")).arg(index.row() + 1), this, [this, index]() {
            AOCtrlEmit(0xFF0B, index.row() | 0xA500);
        });
        myMenu->addAction(QString("%1:BMU%2").arg(tr("关闭风扇")).arg(index.row() + 1), this, [this, index]() {
            AOCtrlEmit(0xFF0B, index.row() | 0x5A00);
        });
        myMenu->addAction(tr("开启全部风扇"), this, [this]() {
            AOCtrlEmit(0xFF0B, 0xA5FE);
        });
        myMenu->addAction(tr("关闭全部风扇"), this, [this, index]() {
            AOCtrlEmit(0xFF0B, 0x5AFF);
        });
        if (this->mycmu->is_cpVer_with_fan_rate()) {
            myMenu->addAction(tr("使能RTU风扇控制"), this, [this, index]() {
                AOCtrlEmit(0xFF0D, 0xAA55);
                rtu_enable = true;
            });
            myMenu->addAction(tr("禁用RTU风扇控制"), this, [this, index]() {
                AOCtrlEmit(0xFF0D, 0x55AA);
                rtu_enable = false;
            });
            myMenu->addAction(QString("%1:BMU%2").arg(tr("设置转速")).arg(index.row() + 1), this, [this, index]() {

                int bmu_nums = ui->tableBMU->rowCount();
                int bmuNum = 0;
                uint8_t speed;
                bool block = true;

                speed = myHelper::showInputBox(tr("风扇转速(0-100)"), block).toUInt();

                if (0 <= speed && speed <= 100) {
                    for (int i = 0; i < bmu_nums; ++i) {
                        bmuNum = i + 1;
                        uint16_t temp = 0;
                        TMsgData MsgCmd;
                        MsgCmd.msg_type = CTRL_AO_ADDR;

                        // 修改奇数号bmu风扇转速
                        if ((bmuNum % 2) == 1) {
                            temp = speed << 8;
                            // 判断偶数号bmu风扇转速是否有修改记录
                            if (fan_Speed_map.contains(bmuNum + 1)) {
                                temp |= fan_Speed_map[bmuNum + 1];
                            }
                        } else {
                            temp = speed;
                            // 判断奇数号bmu风扇转速是否有修改记录
                            if (fan_Speed_map.contains(bmuNum - 1)) {
                                temp |= fan_Speed_map[bmuNum - 1] << 8;
                            }
                        }

                        fan_Speed_map[bmuNum] = speed;

                        uint16_t val[2] = {0, 0};
                        if ((bmuNum % 2) == 1) {
                            val[0] = 0xFF0E + (bmuNum + 1) / 2 - 1;
                        } else {
                            val[0] = 0xFF0E + bmuNum / 2 - 1;
                        }

                        val[1] = temp;
                        MsgCmd.data.append(reinterpret_cast<char*>(&val), sizeof(val));
                        if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
                    }
                }else{
                    myHelper::ShowMessageBoxError(tr("转速不在区间[0,100]内"));
                }
            });
        }

        myMenu->move(cursor().pos());
        myMenu->show();
        myMenu->setAttribute(Qt::WA_DeleteOnClose);
        // Show context menu at handling position
        //        myMenu.exec(globalPos);
    }
}

void BMSView::rebootBmus()
{
    TMsgData MsgCmd;
    mb_cmd cmd = {CTRL_CMD_REBOOT, ADDR_REBOOT, MB_REBOOT_BMU};
    MsgCmd.msg_type = cmd.type;
    MsgCmd.data.append(reinterpret_cast<char*>(&cmd.addr), sizeof(uint16_t));
    MsgCmd.data.append(reinterpret_cast<char*>(&cmd.value), sizeof(uint16_t));
    if (MsgCmd.data.size() > 0) emit send_msg(MsgCmd);
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

void BMSView::on_btn_debugLog_clicked()
{
    savelog.show();
}





void BMSView::on_le_ErrLogUcellLimit_editingFinished()
{
    bool ok;
    uint data = ui->le_ErrLogUcellLimit->text().toUInt(&ok,10);
    if(ok){
        settings->setValue("global/le_ErrLogUcellLimit", data);
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_SET_ERRLOG_ULIMIT;
        MsgCmd.data.setNum(data);
        emit send_msg(MsgCmd);
        MsgCmd.data.clear();
    }
}

void BMSView::on_le_ErrLogTempLimit_editingFinished()
{
    bool ok;
    uint data = ui->le_ErrLogTempLimit->text().toUInt(&ok,10);
    if(ok){
        settings->setValue("global/le_ErrLogTempLimit", data);
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_SET_ERRLOG_TLIMIT;
        MsgCmd.data.setNum(data);
        emit send_msg(MsgCmd);
        MsgCmd.data.clear();
    }
}

void BMSView::on_le_ErrLog_StdValue_editingFinished()
{
    bool ok;
    uint data = ui->le_ErrLog_StdValue->text().toUInt(&ok,10);
    if(ok){
        settings->setValue("global/le_ErrLog_StdValue", data);
        TMsgData MsgCmd;
        MsgCmd.msg_type = CTRL_SET_ERRLOG_STDVAL;
        MsgCmd.data.setNum(data);
        emit send_msg(MsgCmd);
        MsgCmd.data.clear();
    }
}

void BMSView::radioBtnToggledChanged(bool arg)
{
    if(arg == false)return;

    TMsgData MsgCmd;
    if(ui->rb_ErrLog_OnceWrite->isChecked()){
        MsgCmd.msg_type = CTRL_SET_ERRLOG_METHOD;
        MsgCmd.data.setNum(ERRLOG_ONCE);
        emit send_msg(MsgCmd);
    }else if(ui->rb_ErrLog_alwaysWrite->isChecked()){
        MsgCmd.msg_type = CTRL_SET_ERRLOG_METHOD;
        MsgCmd.data.setNum(ERRLOG_ALWAYS);
        emit send_msg(MsgCmd);
    }else if(ui->rb_ErrLog_NtimesWrite->isChecked()){
        MsgCmd.msg_type = CTRL_SET_ERRLOG_METHOD;
        uint16_t value[2] = {0, 0};
        value[0] = ERRLOG_NTIMES;
        value[1] = ui->sb_ErrLog_Count->value();
        MsgCmd.data.append(reinterpret_cast<char*>(&value), 2 * sizeof(uint16_t));
        emit send_msg(MsgCmd);
    }
    MsgCmd.data.clear();
}




void BMSView::on_cB_Func2_1213_currentIndexChanged(int index)
{
    if(index < 4 && index > -1){
        if(index%2 == 0){
            if(ui->bFuncEx12->isChecked()) ui->bFuncEx12->setChecked(false);
        }else{
            if(!ui->bFuncEx12->isChecked()) ui->bFuncEx12->setChecked(true);
        }

        if(index/2 == 0){
            if(ui->bFuncEx13->isChecked()) ui->bFuncEx13->setChecked(false);
        }else{
            if(!ui->bFuncEx13->isChecked()) ui->bFuncEx13->setChecked(true);
        }
    }
}


void BMSView::on_cB_Func2_1415_currentIndexChanged(int index){
    if(index < 4 && index > -1){
        if(index%2 == 0){
            if(ui->bFuncEx14->isChecked()) ui->bFuncEx14->setChecked(false);
        }else{
            if(!ui->bFuncEx14->isChecked()) ui->bFuncEx14->setChecked(true);
        }

        if(index/2 == 0){
            if(ui->bFuncEx15->isChecked()) ui->bFuncEx15->setChecked(false);
        }else{
            if(!ui->bFuncEx15->isChecked()) ui->bFuncEx15->setChecked(true);
        }
    }
}


void BMSView::on_refreashIp_clicked()
{
    ui->refreashIp->setEnabled(false);
    QString btnStr = ui->refreashIp->text();
    ui->refreashIp->setText(btnStr + ".");
    for(int i = 1; i < ui->cbConnectIP->count(); i++){
        ui->cbConnectIP->removeItem(i);
    }


    QString ip123 = "192.168.1.";
    ui->spinBoxPort->setValue(502);
    for(int ip4 = 120; ip4 <= 140; ip4++ ){
        QString ip = ip123 + QString::number(ip4);
        QTcpSocket tcpClient;
        tcpClient.abort();
//        tcpClient.connectToHost(ip, ui->spinBoxPort->value());
//        //200毫秒没有连接上则判断不在线
//        if(tcpClient.waitForConnected(200)){
//            ui->cbConnectIP->addItem(ip);
//            ui->cbConnectIP->setCurrentText(ip);
//        }
        tcpClient.connectToHost(ip, ui->spinBoxPort->value());

        QEventLoop loop;
        QTimer timer;

        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        QObject::connect(&tcpClient, &QTcpSocket::connected, &loop, &QEventLoop::quit);

        timer.start(200);
        loop.exec();

        if (tcpClient.state() == QAbstractSocket::ConnectedState) {
            ui->cbConnectIP->addItem(ip);
            ui->cbConnectIP->setCurrentText(ip);
            Toast::showTip(QString(tr("添加ip：%1 到下拉选项").arg(ip)), nullptr);
        }

        if(ip4 == 123){
            ui->refreashIp->setText(btnStr + "..");
        }
        if(ip4 == 126){
            ui->refreashIp->setText(btnStr + "...");
        }
    }

    ui->refreashIp->setText(btnStr);
    ui->refreashIp->setEnabled(true);
    if(myHelper::IsIP(ui->cbConnectIP->currentText())){
        ui->tbtnConnect->clicked();
    }

}

static uint32_t verToUint(QString str){
    QList<QString> strList = str.split('.', QString::SkipEmptyParts);
    uint32_t res = 0;
    for(int i = 0; i < 4; i++){
        bool ok;
        res += ((strList.at(i).toUInt(&ok,16))&0xFF)<<((3-i)*8);
    }
    return res;
}


void BMSView::findPreVer(void){
    if(this->mycmu->cmu_ver == verCache){
        return;
    }
    verCache = this->mycmu->cmu_ver;

    QString errStr = tr("没找到推荐的版本");
    QString okStr  = tr("推荐版本：");

    QFile file(":/CMUver.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打开文件失败";
        ui->labelPreferrVer->setText(errStr);
        return;
    }
    QTextStream in(&file);
    QMap<QString,QString> verMap;
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
//        qDebug() << line;
        QList<QString> lineList = line.split(':', QString::SkipEmptyParts);
        if(lineList.count() == 2){
            QString CMUver = lineList.at(0);
            QString ver    = lineList.at(1);
            CMUver.remove(" ");
            ver.remove(" ");

            verMap[CMUver] = ver;
        }

    }

//    qDebug() << verMap;

    file.close();

    QMap<QString,QString>::iterator itor;
    for (itor = verMap.begin(); itor != verMap.end(); ++itor)
    {
        QString CMUver = itor.key();
        QString ver    = itor.value();

        QList<QString> verList = ver.split(',', QString::SkipEmptyParts);
        foreach(QString verMinMax, verList){
            QString verMin = verMinMax.split('-', QString::SkipEmptyParts).at(0);
            QString verMax = verMinMax.split('-', QString::SkipEmptyParts).at(1);

            uint32_t u32VerMin = verToUint(verMin);
            uint32_t u32VerMax = verToUint(verMax);

            QString strCmuProVer = "";
            QString strCmuExver  = "";

            uint16_t u16CmuProVer = 0;
            uint16_t u16CmuExver  = 0;

            u16CmuProVer = CMUver.toInt()/1000;
            u16CmuExver  = CMUver.toInt()%1000;

            switch(BMS_PROTOCOL(u16CmuProVer))
            {
                case CMU_V0:
                    strCmuProVer = "CMU_V0";
                break;

                case CMU_V1:
                    strCmuProVer = "CMU_V1";
                break;

                case CMU_V2:
                    strCmuProVer = "CMU_V2";
                break;

                case CMU_V3:
                    strCmuProVer = "CMU_V3";
                break;

                default:
                    strCmuProVer = "CMU_V0";
                break;
            }
            strCmuExver  = QString("_%1").arg(u16CmuExver,3,10,QChar('0'));

            if( (verCache >= u32VerMin) && (verCache <= u32VerMax) ){
                okStr += strCmuProVer+strCmuExver;
                qDebug() << QString("最合适的版本范围：%1 - %2  => %3")
                            .arg(verMin)
                            .arg(verMax)
                            .arg(verCache,8,16,QChar('0'));
                ui->labelPreferrVer->setText(okStr);

                uint16_t cp = u16CmuProVer*1000 + u16CmuExver;
                this->mycmu->setCompoundProtocolVer(cp);
                qDebug() << "对应CMU协议号："
                         << this->mycmu->compound_protocol_ver()
                         << this->mycmu->GetProtocalVer()
                         << this->mycmu->GetExProtocalVer();
                ui->cbProtocol->setCurrentIndex(u16CmuProVer);
                ui->sysStatWd->setProtocol(cp);

//                TMsgData MsgCmd;
//                MsgCmd.msg_type = CTRL_SET_EXPRO;
//                MsgCmd.data.setNum(cp);
//                emit send_msg(MsgCmd);
                return;
            }
        }

    }
    ui->labelPreferrVer->setText(errStr);


}



void BMSView::on_pb_clearNetErrCnt_clicked()
{
    ui->sp_netErrCnt->setValue(0);
}






void BMSView::on_pb_modBusHelp_clicked()
{
    QFile file1(":/verHelp.txt");
    if (!file1.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打开文件失败";
        return;
    }
    QString strFile1 = file1.readAll();

    QFile file2(":/CMUver.txt");
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打开文件失败";
        return;
    }
    QString strFile2 = file2.readAll();
    ProtocolSetFrame* psf = new ProtocolSetFrame;
    psf->setText(strFile1 + strFile2);
    psf->setCPVer(this->mycmu->compound_protocol_ver());
    psf->setVerList(this->mycmu->getCPVerList());
    connect(this->mycmu, &mb_cmu::cpVerChanged, psf, &ProtocolSetFrame::setCPVer);
    connect(psf, &ProtocolSetFrame::setCPVerConfirm, this->mycmu, &mb_cmu::setCompoundProtocolVer);

    psf->exec();
    disconnect(this->mycmu, &mb_cmu::cpVerChanged, psf, &ProtocolSetFrame::setCPVer);


}

void BMSView::lbListInit()
{
    lbSysStatList.clear();
    lbSysStat2List.clear();
    lbDIStatList.clear();
    lbDOStatList.clear();
    lbErrStatList.clear();
    lbErrStat2List.clear();
    lbAlmStatList.clear();
    lbAlmStat2List.clear();
    lbWarmStatList.clear();

    lbSysStatList << ui->bSysErr << ui->bSysAlm << ui->bSysFull << ui->bSysEmpty
                  << ui->bSysInit << ui->bSysCommErr << ui->bSysBalance << ui->bSysCharge
                  << ui->bSysDischarge << ui->bSysStop << ui->bSys10 << ui->bSys11
                  << ui->bSys12 << ui->bSys13 << ui->bSys14 << ui->bSys15;

    lbSysStat2List<< ui->bSysStat2_1 << ui->bSysStat2_2 << ui->bSysStat2_3 << ui->bSysStat2_4
                  << ui->bSysStat2_5 << ui->bSysStat2_6 << ui->bSysStat2_7 << ui->bSysStat2_8
                  << ui->bSysStat2_9 << ui->bSysStat2_10<< ui->bSysStat2_11 << ui->bSysStat2_12
                  << ui->bSysStat2_13 << ui->bSysStat2_14 << ui->bSysStat2_15 << ui->bSysStat2_16;

    lbDIStatList  << ui->bDI0 << ui->bDI1 << ui->bDI2 << ui->bDI3
                  << ui->bDI4 << ui->bDI5 << ui->bDI6 << ui->bDI7
                  << ui->bDI8 << ui->bDI9 << ui->bDI10 << ui->bDI11
                  << ui->bDI12 << ui->bDI13 << ui->bDI14 << ui->bDI15;

    lbDOStatList  << ui->bDO0 << ui->bDO1 << ui->bDO2 << ui->bDO3
                  << ui->bDO4 << ui->bDO5 << ui->bDO6 << ui->bDO7
                  << ui->bDO8 << ui->bDO9 << ui->bDO10 << ui->bDO11
                  << ui->bDO12 << ui->bDO13 << ui->bDO14 << ui->bDO15;

    lbErrStatList << ui->bErr0 << ui->bErr1 << ui->bErr2 << ui->bErr3
                  << ui->bErr4 << ui->bErr5 << ui->bErr6 << ui->bErr7
                  << ui->bErr8 << ui->bErr9 << ui->bErr10 << ui->bErr11
                  << ui->bErr12 << ui->bErr13 << ui->bErr14 << ui->bErr15;

    lbErrStat2List<< ui->bErr0_2 << ui->bErr1_2 << ui->bErr2_2 << ui->bErr3_2
                  << ui->bErr4_2 << ui->bErr5_2 << ui->bErr6_2 << ui->bErr7_2
                  << ui->bErr8_2 << ui->bErr9_2 << ui->bErr10_2 << ui->bErr11_2
                  << ui->bErr12_2 << ui->bErr13_2 << ui->bErr14_2 << ui->bErr15_2;

    lbAlmStatList << ui->bAlm0 << ui->bAlm1 << ui->bAlm2 << ui->bAlm3
                  << ui->bAlm4 << ui->bAlm5 << ui->bAlm6 << ui->bAlm7
                  << ui->bAlm8 << ui->bAlm9 << ui->bAlm10 << ui->bAlm11
                  << ui->bAlm12 << ui->bAlm13 << ui->bAlm14 << ui->bAlm15;

    lbAlmStat2List<< ui->bAlm0_2 << ui->bAlm1_2 << ui->bAlm2_2 << ui->bAlm3_2
                  << ui->bAlm4_2 << ui->bAlm5_2 << ui->bAlm6_2 << ui->bAlm7_2
                  << ui->bAlm8_2 << ui->bAlm9_2 << ui->bAlm10_2 << ui->bAlm11_2
                  << ui->bAlm12_2 << ui->bAlm13_2 << ui->bAlm14_2 << ui->bAlm15_2;

    lbWarmStatList<< ui->bPreAlm0 << ui->bPreAlm1 << ui->bPreAlm2 << ui->bPreAlm3
                  << ui->bPreAlm4 << ui->bPreAlm5 << ui->bPreAlm6 << ui->bPreAlm7
                  << ui->bPreAlm8 << ui->bPreAlm9 << ui->bPreAlm10 << ui->bPreAlm11
                  << ui->bPreAlm12 << ui->bPreAlm13 << ui->bPreAlm14 << ui->bPreAlm15;

}


void BMSView::fillStatLabel(QList<QLabel*> &ll, uint16_t value, QStringList sl)
{
    foreach (QLabel* Label, ll) {
        bool flag = ((value >> ll.indexOf(Label)) & 0x01) > 0;
        QString color = flag ? TEXT_RED : TEXT_GREEN;
        Label->setStyleSheet(QString("%1").arg(color));
        Label->setText(sl.at(ll.indexOf(Label)));

        ui->sysStatWd->setLabel(Label->text(),flag);//guest界面会用到
    }
}


void BMSView::clearStatLabel(QList<QLabel *> &ll)
{
    foreach (QLabel* Label, ll) {
        Label->setText(EMPTY_TEXT);
    }
}



