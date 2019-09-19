#pragma execution_character_set("utf-8")
#include "main_ui.h"

#include <QTimer>
#include "iconhelper.h"
#include "ui_main_ui.h"

MainUI::MainUI(QWidget* parent) : QWidget(parent), ui(new Ui::MainUI) {
    ui->setupUi(this);
    this->initForm();
    this->initLeftMain();
    this->initLeftConfig();
    this->pcmu = new mb_cmu;
    pmq = MessageQueue::getInstance();
    TMsgData MsgCmd;
    MsgCmd.msg_type = CONFIG_IP;
    QString ip = ui->lineEditIP->text();
    MsgCmd.data.append(ip);
    pmq->sendMsg(0,MsgCmd);
    this->pcmu->start();
    ui->cmuData->mycmu = pcmu;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpDate()));
    //timer->start(1000);
}

MainUI::~MainUI() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    pmq->sendMsg(0,MsgCmd);
    pcmu->wait();
    delete pcmu;
    delete ui;
}

void MainUI::initForm() {
    this->setProperty("form", true);
    this->setProperty("canMove", true);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf073), 30);
    IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
    IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf067));
    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));

    // ui->widgetMenu->setVisible(false);
    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTop->setProperty("nav", "top");
    ui->labTitle->setText("库博BMS监控软件");
    ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
    this->setWindowTitle(ui->labTitle->text());

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

    // ui->widgetLeftMain->setProperty("flag", "left");
    ui->widgetLeftConfig->setProperty("flag", "left");
    ui->MainPage->setStyleSheet(QString("QWidget[flag=\"left\"] "
                                        "QAbstractButton{min-height:%1px;max-height:%1px;}")
                                    .arg(60));
    ui->page2->setStyleSheet(QString("QWidget[flag=\"left\"] "
                                     "QAbstractButton{min-height:%1px;max-height:%1px;}")
                                 .arg(20));
}

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
    MessageQueue* pmq = MessageQueue::getInstance();
    TMsgData MsgCmd;
    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
        MsgCmd.msg_type = 0;
        QString ip = ui->lineEditIP->text();
        MsgCmd.data.append(ip);
        bool ret = pmq->sendMsg(0,MsgCmd);
        qDebug() << "send "<<MsgCmd.msg_type << "," << MsgCmd.data<<","<<ret<<","<<MsgCmd.data.size();
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

    //    QList<QSpinBox*> tbtns = ui->widgetTop->findChildren<QSpinBox*>();
    //    foreach (QSpinBox* btn, tbtns) {
    //        if (btn == b) {
    //            btn->setChecked(true);
    //        } else {
    //            btn->setChecked(false);
    //        }
    //    }
//    MessageQueue* pmq = MessageQueue::getInstance();
//    TMsgData MsgCmd;
//    if (name == "主界面") {
//        ui->stackedWidget->setCurrentIndex(0);
//    } else if (name == "系统设置") {
//        ui->stackedWidget->setCurrentIndex(1);
//    } else if (name == "事件查询") {
//        ui->stackedWidget->setCurrentIndex(2);
//    } else if (name == "使用帮助") {
//        ui->stackedWidget->setCurrentIndex(3);
//    } else if (name == "用户退出") {
//        exit(0);
//    }
}

void MainUI::initLeftMain() {
    pixCharMain << 0xf030 << 0xf03e << 0xf247;
    // btnsMain << ui->tbtnMain1 << ui->tbtnMain2 << ui->tbtnMain3;

    int count = btnsMain.count();
    for (int i = 0; i < count; i++) {
        btnsMain.at(i)->setCheckable(true);
        btnsMain.at(i)->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(btnsMain.at(i), SIGNAL(clicked(bool)), this, SLOT(leftMainClick()));
    }

    // IconHelper::Instance()->setStyle(ui->widgetLeftMain, btnsMain, pixCharMain, 15, 35, 25, "left", 4);
    connect(ui->tbtnConnect, SIGNAL(clicked(bool)), this, SLOT(btnClick()));
    // ui->tbtnMain1->click();
    // ui->listBMS->set
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
        string ip = ui->lineEditIP->text().toStdString();
        int port = ui->spinBoxPort->value();
        this->pcmu->Init();
        qDebug() << "connect " << ui->lineEditIP->text();
    }
}
void MainUI::on_btnMenu_Min_clicked() { showMinimized(); }

void MainUI::on_btnMenu_Max_clicked() {
    static bool max = false;
    static QRect location = this->geometry();

    if (max) {
        this->setGeometry(location);
    } else {
        location = this->geometry();
        this->setGeometry(qApp->desktop()->availableGeometry());
    }

    this->setProperty("canMove", max);
    max = !max;
}

void MainUI::on_btnMenu_Close_clicked() { close(); }

void MainUI::timerUpDate() {
    if(pcmu == nullptr) return;
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
    ui->tableConfig->setRowCount(this->pcmu->max_offset);
    ui->tableConfig->setColumnCount(1);
    for (int i = 0; i < this->pcmu->max_offset; i++) {
        QTableWidgetItem* item = new QTableWidgetItem();
        double val = this->pcmu->tab_reg[i];
        item->setText(QString("%1").arg(val, 0, 'g', 5));
        //      item->setBackground(QBrush(QColor(Qt::lightGray)));
        //      item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        ui->tableConfig->setItem(i, 0, item);
    }
    // elapsed(): 返回自上次调用start()或restart()以来经过的毫秒数
    // qDebug() << t.elapsed() << "ms";
}
