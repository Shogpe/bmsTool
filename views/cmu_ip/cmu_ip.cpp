#include "cmu_ip.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include "iconhelper.h"
#include "myhelper.h"
#include "ui_cmu_ip.h"
#include "Toast.h"
CmuIpView::CmuIpView(QWidget* parent) : QTabWidget(parent), ui(new Ui::CmuIpView) {
    ui->setupUi(this);
    this->timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CmuIpView::timerUpDate);
    timer->start(500);
    {
      QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
      settings = new QSettings("config.ini", QSettings::IniFormat);
      QString target_ip = settings->value("global/target_ip", "192.168.1.120").toString();
      ui->lineStaIP->setText(target_ip);
      QString protocol = settings->value("global/protocol", "CMU").toString();
      ui->cbProtocol->setCurrentText(protocol);
      if (protocol == "CMU") {
        this->mycmu = new mb_cmu(CMUV1);
        ui->cbProtocol->blockSignals(true);
        ui->cbProtocol->setCurrentIndex(CMUV1);
        ui->cbProtocol->blockSignals(false);
      } else {
        this->mycmu = new mb_cmu(CMUV2);
        ui->cbProtocol->blockSignals(true);
        ui->cbProtocol->setCurrentIndex(CMUV2);
        ui->cbProtocol->blockSignals(false);
      }
      delete settings;
    }
    pmq = MessageQueue::getInstance();
    pmq->registMsgQueue(99);
    config = {0, 0, 0, 0, 0};
//    this->mycmu->start();
    connect(ui->lineStaIP, &QLineEdit::editingFinished, this, &CmuIpView::IpChange, Qt::UniqueConnection);
    connect(this->mycmu, static_cast<void (mb_cmu::*)(const QString&)>(&mb_cmu::signal_message), this,
            static_cast<void (CmuIpView::*)(const QString&)>(&CmuIpView::slot_message_call), Qt::UniqueConnection);
    initUpdateMenu();

}

CmuIpView::~CmuIpView() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    pmq->sendMsg(0, MsgCmd);
//    this->mycmu->wait();
    timer->stop();
    delete timer;
    delete ui;
}
void CmuIpView::slot_message_call(const QString& msg) {
  Toast::showTip(msg, nullptr);
}
void CmuIpView::uiInit() {

}
void CmuIpView::timerUpDate() {
    if ( mycmu == nullptr) return;
    if (this->mycmu->drv_status) {
      ui->labelStatus->setStyleSheet("color:green");
      ui->labelStatus->setText(tr("已连接"));
      if (this->mycmu->drv_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("软件过期，请更新！"));
      uint32_t val = this->mycmu->cmu_ver;
      ui->labelVer->setText(tr("版本号:%1").arg(myHelper::IntegerToHexString(val)));
      ui->tbtnConnect->setText(tr("重连"));
    } else {
      ui->labelStatus->setStyleSheet("color:red");
      ui->labelStatus->setText(tr("未连接"));
      ui->tbtnConnect->setText(tr("连接"));
    }
    flushData();
}

void CmuIpView::flushData() {
    if (!ui->lineTftpIP->hasFocus())
        ui->lineTftpIP->setText(myHelper::IPV4IntegerToString(mycmu->sys_para.Name.u32TftpServIP));
    if (!ui->lineBmsIP->hasFocus())
        ui->lineBmsIP->setText(myHelper::IPV4IntegerToString(mycmu->sys_para.Name.u32LocalIP));
}

void CmuIpView::on_lineBmsIP_editingFinished() {
    QLineEdit* pEdit = ui->lineBmsIP;
    QString ip_str = pEdit->text();
    if (!pEdit->isModified()) return;
    pEdit->setModified(false);
    if (!myHelper::IsIP(ip_str)) {
        myHelper::ShowMessageBoxError(tr("invalid ip address!"));
        pEdit->undo();
        return;
    }
    this->setFocus();
    if (myHelper::ShowMessageBoxQuesion(QString(tr("确定要修改设备IP为%1吗").arg(ip_str))) != QDialog::Accepted) {
        pEdit->undo();
        return;
    }
    uint32_t ip = myHelper::IPV4StringToInteger(ip_str);
    if((ip>>16 & 0xFFFF) != 0xC0A8){
      myHelper::ShowMessageBoxError(tr("网段必须为192.168.x.x"));
      pEdit->undo();
      return;
    }
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

void CmuIpView::on_lineTftpIP_editingFinished() {
    QLineEdit* pEdit = ui->lineTftpIP;
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

void CmuIpView::on_tbtnConnect_released()
{
  QToolButton* b = (QToolButton*)sender();
  QString name = b->text();
  if (name == "连接" || name == "重连") {
    uint16_t port = ui->spinBoxPort->value();
    TMsgData MsgCmd;
    MsgCmd.msg_type = CONFIG_IP;
    MsgCmd.data.append(ui->lineStaIP->text());
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

void CmuIpView::on_cbProtocol_currentIndexChanged(const QString &arg1)
{
  QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
  settings->setValue("global/protocol", arg1);
  TMsgData MsgCmd;
  if (arg1 == "CMU") {
    MsgCmd.msg_type = CTRL_SET_PRO;
    MsgCmd.data.setNum(CMUV1);
  } else {
    MsgCmd.msg_type = CTRL_SET_PRO;
    MsgCmd.data.setNum(CMUV1);
  }
  pmq->sendMsg(0, MsgCmd);
  MsgCmd.data.clear();
  delete settings;
}
void CmuIpView::initUpdateMenu() {
  update_menu = new QMenu;
  update_menu->addAction("下载升级BMS", this, &CmuIpView::onUpdateBtnMenu);
//  update_menu->addAction("下载升级BMU", this, &Widget::onUpdateBtnMenu);
//  update_menu->addAction("下载升级BMS Boot", this, &Widget::onUpdateBtnMenu);
//  update_menu->addAction("下载升级BMU Boot", this, &Widget::onUpdateBtnMenu);
//  update_menu->addAction("下载升级绝缘板", this, &Widget::onUpdateBtnMenu);
//  update_menu->addAction("升级BMU", this, &Widget::onUpdateBtnMenu);
  ui->tbtnUpdate->setMenu(update_menu);
}

void CmuIpView::onUpdateBtnMenu() {
  QAction* b = (QAction*)sender();
  TMsgData MsgCmd;
  if (b->text() == "下载升级BMS") {
    MsgCmd.msg_type = CTRL_SEC_AO;
    uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateCMU};
    MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
  }/* else if (b->text() == "下载升级BMU") {
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
  }*/ else {
    return;
  }
  pmq->sendMsg(0, MsgCmd);
  MsgCmd.data.clear();
  return;
}

void CmuIpView::IpChange() {
  QLineEdit* pEdit = (QLineEdit*)sender();
  qDebug()<<"ip edit fin.";
  if (!pEdit->isModified()) return;
  pEdit->setModified(false);
  QString ip = pEdit->text();
  QRegExp RegExp("(192.168.(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.)(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
  if (!RegExp.exactMatch(ip)) {
    myHelper::ShowMessageBoxError(tr("非法地址!网段必须为192.168.x.x"));
    pEdit->undo();
    return;
  }
  TMsgData MsgCmd;
  MsgCmd.msg_type = CONFIG_IP;
  MsgCmd.data.append(ip);
  pmq->sendMsg(0, MsgCmd);
  QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
  settings->setValue("global/target_ip", ip);
  delete settings;
}
