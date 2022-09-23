#include "rtu_tool.h"
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QtXml>
#include "Toast.h"
#include "iconhelper.h"
#include "myhelper.h"
#include "ui_rtu_tool.h"
RTUView::RTUView(QWidget* parent) : QTabWidget(parent), ui(new Ui::RTUView) {
    ui->setupUi(this);
    this->timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RTUView::timerUpDate);
    timer->start(500);
    if (0) {
        QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
        settings = new QSettings("config.ini", QSettings::IniFormat);
        QString target_ip = settings->value("global/target_ip", "192.168.1.120").toString();
        ui->lineStaIP->setText(target_ip);
        QString protocol = settings->value("global/protocol", "CMU").toString();
        ui->cbProtocol->setCurrentText(protocol);
        if (protocol == "CMU") {
            this->m_device = new mb_cmu(CMUV1);
            ui->cbProtocol->blockSignals(true);
            ui->cbProtocol->setCurrentIndex(CMUV1);
            ui->cbProtocol->blockSignals(false);
        } else {
            this->m_device = new mb_cmu(CMUV2);
            ui->cbProtocol->blockSignals(true);
            ui->cbProtocol->setCurrentIndex(CMUV2);
            ui->cbProtocol->blockSignals(false);
        }
        delete settings;
    }
    pmq = MessageQueue::getInstance();
    pmq->registMsgQueue(99);
    config = {0, 0, 0, 0, 0};
    this->m_device->start();
    connect(ui->lineStaIP, &QLineEdit::editingFinished, this, &RTUView::IpChange, Qt::UniqueConnection);
    connect(this->m_device, static_cast<void (mb_cmu::*)(const QString&)>(&mb_cmu::signal_message), this,
            static_cast<void (RTUView::*)(const QString&)>(&RTUView::slot_message_call), Qt::UniqueConnection);
    uiInit();
}

RTUView::~RTUView() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    pmq->sendMsg(0, MsgCmd);
    this->m_device->wait();
    timer->stop();
    delete timer;
    delete ui;
}
void RTUView::slot_message_call(const QString& msg) { Toast::showTip(msg, nullptr); }
void RTUView::uiInit() {
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
}
void RTUView::timerUpDate() {
    if (m_device == nullptr) return;
    if (this->m_device->drv_status) {
        ui->labelStatus->setStyleSheet("color:green");
        ui->labelStatus->setText(tr("已连接"));
        if (this->m_device->drv_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("软件过期，请更新！"));
        ui->tbtnConnect->setText("重连");
    } else {
        ui->labelStatus->setStyleSheet("color:red");
        ui->labelStatus->setText(tr("未连接"));
        ui->tbtnConnect->setText("连接");
    }
    flushData();
}

void RTUView::flushData() {

}

void RTUView::on_lineBmsIP_editingFinished() {

}

void RTUView::on_lineTftpIP_editingFinished() {

}

void RTUView::on_tbtnConnect_released() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->text();

}

void RTUView::on_cbProtocol_currentIndexChanged(const QString& arg1) {

}
void RTUView::initUpdateMenu() {

}

void RTUView::onUpdateBtnMenu() {
    QAction* b = (QAction*)sender();
    TMsgData MsgCmd;
    if (b->text() == "下载升级BMS") {
        MsgCmd.msg_type = CTRL_SEC_AO;
        uint16_t val[2] = {ADDR_UPGRADE, MB_UpdateCMU};
        MsgCmd.data.append((char*)(&val), 2 * sizeof(uint16_t));
    } /* else if (b->text() == "下载升级BMU") {
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
     }*/
    else {
        return;
    }

    return;
}

void RTUView::IpChange() {

}
