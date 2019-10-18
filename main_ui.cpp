#pragma execution_character_set("utf-8")
#include "main_ui.h"
#include <QTimer>
#include "iconhelper.h"
#include "ui_main_ui.h"
#include "version.h"

MainUI::MainUI(QWidget* parent) : QFramelessWidget(parent), ui(new Ui::MainUI) {
    ui->setupUi(this);
    this->initForm();
    this->initLeftMain();
    this->initLeftConfig();
    this->pcmu = new mb_cmu;
    pmq = MessageQueue::getInstance();
    TMsgData MsgCmd;
    MsgCmd.msg_type = CONFIG_IP;
    ui->lineEditIP->setText("192.168.1.120");
    QString ip = ui->lineEditIP->text();
    MsgCmd.data.append(ip);
    pmq->sendMsg(0, MsgCmd);
    this->pcmu->start();
    ui->cmuData->mycmu = pcmu;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpDate()));
    timer->start(1000);
}

MainUI::~MainUI() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    pmq->sendMsg(0, MsgCmd);
    pcmu->wait();
    delete pcmu;
    delete ui;
}

void MainUI::initForm() {
    this->setProperty("form", true);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf073), 40);
    IconHelper::Instance()->setIcon(ui->btnMenu, QChar(0xf00b));
    IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
    IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf067));
    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));

    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTop->setProperty("nav", "top");
    ui->labTitle->setText("库博BMS监控软件");
    ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
    this->setWindowTitle(ui->labTitle->text());
    ui->labVersion->setText(QString("battery management system v") + VER_PRODUCTVERSION_STR);
    ui->labUser->setText("Ganing");
    // ui->stackedWidget->setStyleSheet("QLabel{font:60pt;}");

    QSize icoSize(32, 32);
    int icoWidth = 85;

    //设置顶部导航按钮
    QList<QToolButton*> tbtns = ui->widgetTop->findChildren<QToolButton*>();
    foreach (QToolButton* btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }

    ui->btnMain->click();
    ui->widgetLeftConfig->setProperty("flag", "left");
    ui->MainPage->setStyleSheet(QString("QWidget[flag=\"left\"] "
                                        "QAbstractButton{min-height:%1px;max-height:%1px;}")
                                    .arg(60));
    ui->page2->setStyleSheet(QString("QWidget[flag=\"left\"] "
                                     "QAbstractButton{min-height:%1px;max-height:%1px;}")
                                 .arg(20));

    // ui->listBMS->setModel(&list_model);
    // list_model.insertRows(0,1,QModelIndex());
    ui->treeBMS->addItem("192.168.1.120");
    ui->treeBMS->addItem("192.168.1.121");
    ui->treeBMS->addItem("192.168.1.122");
    ui->treeBMS->addItem("192.168.1.123");
    ui->treeBMS->addItem("192.168.1.120","502");
    ui->treeBMS->addItem("192.168.1.120","CMU");
    ui->treeBMS->addItem("192.168.1.121","502");
    ui->treeBMS->addItem("192.168.1.122","502");
    ui->treeBMS->addItem("192.168.1.123","502");
}
#include "Toast.h"
void MainUI::buttonClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->text();

    QList<QToolButton*> tbtns = ui->widgetTop->findChildren<QToolButton*>();
    foreach (QToolButton* btn, tbtns) {
        if (btn == b) {
            btn->setChecked(true);
        } else {
            btn->setChecked(false);
        }
    }
    Toast::showTip(name, nullptr);
    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "系统设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "事件查询") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "使用帮助") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "用户退出") {
        exit(0);
    }
}

void MainUI::valueChange() {
    QSpinBox* b = (QSpinBox*)sender();
    QString name = b->text();
}

void MainUI::initLeftMain() {
    pixCharMain << 0xf030 << 0xf03e << 0xf247;
    int count = btnsMain.count();
    for (int i = 0; i < count; i++) {
        btnsMain.at(i)->setCheckable(true);
        btnsMain.at(i)->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(btnsMain.at(i), SIGNAL(clicked(bool)), this, SLOT(leftMainClick()));
    }
    connect(ui->tbtnConnect, SIGNAL(clicked(bool)), this, SLOT(btnClick()));
}

void MainUI::initLeftConfig() {
    pixCharConfig << 0xf031 << 0xf036 << 0xf249 << 0xf055 << 0xf05a << 0xf249;
    btnsConfig << ui->tbtnConfig1 << ui->tbtnConfig2 << ui->tbtnConfig3 << ui->tbtnConfig4 << ui->tbtnConfig5
               << ui->tbtnConfig6;

    int count = btnsConfig.count();
    for (int i = 0; i < count; i++) {
        btnsConfig.at(i)->setCheckable(true);
        btnsConfig.at(i)->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        connect(btnsConfig.at(i), SIGNAL(clicked(bool)), this, SLOT(leftConfigClick()));
    }

    IconHelper::Instance()->setStyle(ui->widgetLeftConfig, btnsConfig, pixCharConfig, 10, 20, 15, "left", 5);

    ui->tbtnConfig1->click();
}

void MainUI::leftMainClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->text();

    int count = btnsMain.count();
    for (int i = 0; i < count; i++) {
        if (btnsMain.at(i) == b) {
            btnsMain.at(i)->setChecked(true);
            btnsMain.at(i)->setIcon(QIcon(IconHelper::Instance()->getPixmap(btnsMain.at(i), false)));
        } else {
            btnsMain.at(i)->setChecked(false);
            btnsMain.at(i)->setIcon(QIcon(IconHelper::Instance()->getPixmap(btnsMain.at(i), true)));
        }
    }

    // ui->lab1->setText(name);
}

void MainUI::leftConfigClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->text();

    int count = btnsConfig.count();
    for (int i = 0; i < count; i++) {
        if (btnsConfig.at(i) == b) {
            btnsConfig.at(i)->setChecked(true);
            btnsConfig.at(i)->setIcon(QIcon(IconHelper::Instance()->getPixmap(btnsConfig.at(i), false)));
        } else {
            btnsConfig.at(i)->setChecked(false);
            btnsConfig.at(i)->setIcon(QIcon(IconHelper::Instance()->getPixmap(btnsConfig.at(i), true)));
        }
    }
    qDebug() << name;
    if (name == "其他设置") {
    }
}

void MainUI::btnClick() {
    QToolButton* b = (QToolButton*)sender();
    QString name = b->text();
    if (name == "连接" || name == "重连") {
        uint16_t port = ui->spinBoxPort->value();
        TMsgData MsgCmd;
        MsgCmd.msg_type = CONFIG_IP;
        MsgCmd.data.append(ui->lineEditIP->text());
        pmq->sendMsg(0, MsgCmd);
        MsgCmd.msg_type = CONFIG_PORT;
        MsgCmd.data.clear();
        MsgCmd.data.reserve(sizeof(port));
        memcpy(MsgCmd.data.data(), &port, sizeof(port));
        pmq->sendMsg(0, MsgCmd);
    }
}
void MainUI::on_btnMenu_Min_clicked() { showMinimized(); }

void MainUI::on_btnMenu_Max_clicked() {
    static bool max = false;
    // static QRect location = this->geometry();

    if (max) {
        showNormal();
    } else {
        showMaximized();
    }

    // this->setProperty("canMove", max);
    // setMoveEnable(max);
    max = !max;
    // setResizeEnable(max);
}

void MainUI::on_btnMenu_Close_clicked() { close(); }

void MainUI::timerUpDate() {
    ui->labTime->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    if (pcmu == nullptr) return;
    if (this->pcmu->cmu_status) {
        ui->labelStatus->setText(tr("已连接"));
        if (this->pcmu->cmu_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("过期"));
        uint32_t val = this->pcmu->cmu_ver;
        ui->labelVer->setText(QString("版本号:0x%1").arg(uint32_t(val), 8, 16, QLatin1Char('0')));
        ui->tbtnConnect->setText("重连");
    } else {
        ui->labelStatus->setText(tr("未连接"));
        ui->tbtnConnect->setText("连接");
    }
}
