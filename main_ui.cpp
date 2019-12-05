#include "main_ui.h"
#include <QTimer>
#include "Toast.h"
#include "iconhelper.h"
#include "ui_main_ui.h"
#include "version.h"
MainUI::MainUI(QWidget* parent) : QFramelessWidget(parent), ui(new Ui::MainUI) {
    ui->setupUi(this);
    this->initForm();
    this->initLeftMain();
    this->initLeftConfig();
    load_config();
    QString protocol = settings->value("global/protocol", "CMU1.0").toString();
    if (protocol == "CMU2.0") {
        this->pDev = new mb_cmu_v2;
        ui->cbProtocol->blockSignals(true);
        ui->cbProtocol->setCurrentIndex(CMUV2);
        ui->cbProtocol->blockSignals(false);
    } else {
        this->pDev = new mb_cmu;
        ui->cbProtocol->blockSignals(true);
        ui->cbProtocol->setCurrentIndex(CMUV1);
        ui->cbProtocol->blockSignals(false);
    }
    pmq = MessageQueue::getInstance();
    this->pDev->start();
    ui->cmuData->mycmu = pDev;
    connect(ui->lineEditIP, &QLineEdit::editingFinished, this, &MainUI::IpChange, Qt::UniqueConnection);
    connect(pDev, static_cast<void (mb_cmu::*)(const QString&)>(&mb_cmu::signal_message), this,
            static_cast<void (MainUI::*)(const QString&)>(&MainUI::slot_message_call), Qt::UniqueConnection);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpDate()));
    timer->start(500);
}
void MainUI::slot_message_call(const QString& msg) {
    // qDebug() << QString("msg:%1").arg(msg);
    Toast::showTip(msg, nullptr);
}
bool MainUI::load_config() {
    settings = new QSettings("config.ini", QSettings::IniFormat);
    QString target_ip = settings->value("global/target_ip", "192.168.1.120").toString();
    QByteArray ba = settings->value("global/layout").toByteArray();
    ui->lineEditIP->setText(target_ip);
    this->restoreGeometry(ba);
    return true;
}
MainUI::~MainUI() {
    TMsgData MsgCmd;
    MsgCmd.msg_type = THREAD_EXIT;
    pmq->sendMsg(0, MsgCmd);
    pDev->wait();
    QByteArray ba = this->saveGeometry();
    settings->setValue("global/layout", ba);
    delete settings;
    delete pDev;
    delete ui;
}
#include "stategroupbox.h"
void MainUI::initForm() {
    this->setProperty("form", true);
    // this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
    // Qt::WindowMinMaxButtonsHint|Qt::CustomizeWindowHint);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf073), 40);
    IconHelper::Instance()->setIcon(ui->btnMenu, QChar(0xf00b));
    IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
    IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf067));
    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));

    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTitle->installEventFilter(this);
    this->setWidget(this);
    ui->widgetTop->setProperty("nav", "top");
    ui->labTitle->setText("库博BMS监控软件");
    ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
    this->setWindowTitle(ui->labTitle->text());
    ui->labVersion->setText(QString("battery management system v") + VER_PRODUCTVERSION_STR);
    ui->labUser->setText("Ganing");

    QSize icoSize(32, 32);
    int icoWidth = 85;

    //设置顶部导航按钮
    QList<QToolButton*> tbtns = ui->widgetTop->findChildren<QToolButton*>();
    foreach (QToolButton* btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
        // btn->hide();
    }
    //
    ui->gridLayout_3->addWidget(new StateGroupBox());

    ui->btnMain->click();
    //创建语言切换菜单
    langue_menu = new QMenu(tr("Langue"));
    //    setChinese = new QAction(tr("Chinese"), this);
    //    setChinese->setCheckable(true);
    //    setEnglish = new QAction(tr("English"), this);
    //    setEnglish->setCheckable(true);
    //    setEnglish->setChecked(true);
    langue_menu->addAction("Chinese", this, &MainUI::changeLangue);
    langue_menu->addAction("English", this, &MainUI::changeLangue);
    //    langueGroup = new QActionGroup(this);
    //    langueGroup->addAction(setEnglish);
    //    langueGroup->addAction(setChinese);

    //创建主题切换菜单
    theme_menu = new QMenu(tr("Theme"));
    setBlue = new QAction(tr("lightblue"), this);
    setBlue->setCheckable(true);
    setBlue->setChecked(true);
    setBlack = new QAction(tr("psblack"), this);
    setBlack->setCheckable(true);
    setWhite = new QAction(tr("flatwhite"), this);
    setWhite->setCheckable(true);
    theme_menu->addAction(setBlue);
    theme_menu->addAction(setBlack);
    theme_menu->addAction(setWhite);
    themeGroup = new QActionGroup(this);
    themeGroup->addAction(setBlue);
    themeGroup->addAction(setBlack);
    themeGroup->addAction(setWhite);
    //创建主菜单,将主题和语言菜单当二级菜单加入主菜单
    title_menu = new QMenu;
    title_menu->addMenu(langue_menu);
    title_menu->addMenu(theme_menu);
    title_menu->addAction("test", this, &MainUI::changeLangue);
    ui->btnMenu->setMenu(title_menu);  //将主菜单设置到菜单按钮

    //关联换肤和切换语言功能
    ui->btnMenu->setPopupMode(QToolButton::InstantPopup);
    connect(themeGroup, &QActionGroup::triggered, this, &MainUI::changeTheme);
    initUpdateMenu();
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

    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "系统设置") {
        // ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "事件查询") {
        // ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "使用帮助") {
        // ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "重启") {
        qApp->exit(773);
    }
}

void MainUI::IpChange() {
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
void MainUI::changeLangue()  //切换语言
{
    QAction* b = (QAction*)sender();
    qDebug() << b->objectName() << b->text();
    //  if(setChinese->isChecked()){//判断选中了哪个语言
    //    translator->load(":/langue/zh_cn.qm");//加载翻译文件
    //    qApp->installTranslator(translator);//安装翻译文件
    //    //刷新界面,因为没有Ui文件，所以要手动实现刷新,使用Ui文件只需要调用ui->retranslateUi(this)即可
    //    retranslateUI();
    //  }else if(setEnglish->isChecked()){
    //    qApp->removeTranslator(translator);
    //    retranslateUI();
    //  }
}

void MainUI::changeTheme()  //切换主题
{
    if (setBlue->isChecked()) {  //判断选中了哪个主题,然后应用相应主题
        myHelper::SetStyle("lightblue");
    } else if (setBlack->isChecked()) {
        myHelper::SetStyle("psblack");
    } else if (setWhite->isChecked()) {
        myHelper::SetStyle("flatwhite");
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
        MsgCmd.data.append((char*)&port, sizeof(port));
        pmq->sendMsg(0, MsgCmd);
        MsgCmd.msg_type = CONFIG_INIT;
        MsgCmd.data.clear();
        pmq->sendMsg(0, MsgCmd);
    }
}
void MainUI::on_btnMenu_Min_clicked() { showMinimized(); }

void MainUI::on_btnMenu_Max_clicked() {
    static bool max = false;
    if (max) {
        showNormal();
        ui->btnMenu_Max->setToolTip(tr("最大化"));
    } else {
        ui->btnMenu_Max->setToolTip(tr("恢复正常"));
        showMaximized();
    }
    setMoveEnable(max);
    setResizeEnable(max);
    max = !max;
}

void MainUI::on_btnMenu_Close_clicked() { close(); }

void MainUI::timerUpDate() {
    ui->labTime->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    if (pDev == nullptr) return;
    if (this->pDev->cmu_status) {
        ui->labelStatus->setStyleSheet("color:green");
        ui->labelStatus->setText(tr("已连接"));
        if (this->pDev->cmu_status >> CMU_OUTOFDATE) ui->labelStatus->setText(tr("软件过期，请更新！"));
        uint32_t val = this->pDev->cmu_ver;
        ui->btnVer->setText(QString("版本号:%1").arg(myHelper::IntegerToHexString(val)));
        ui->tbtnConnect->setText("重连");
    } else {
        ui->labelStatus->setStyleSheet("color:red");
        ui->labelStatus->setText(tr("未连接"));
        ui->tbtnConnect->setText("连接");
    }
}
bool MainUI::eventFilter(QObject* obj, QEvent* event) {
    if (obj == ui->widgetTitle) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            this->on_btnMenu_Max_clicked();
            return true;
        }
    }
    return QFramelessWidget::eventFilter(obj, event);
}

void MainUI::on_cbProtocol_currentIndexChanged(const QString& arg1) {
    qDebug() << arg1;
    settings->setValue("global/protocol", arg1);
    myHelper::ShowMessageBoxInfo(tr("修改协议，请重启软件方可生效！"));
}

void MainUI::initUpdateMenu() {
    update_menu = new QMenu;
    update_menu->addAction("下载升级BMS", this, &MainUI::onUpdateBtnMenu);
    update_menu->addAction("下载升级BMU", this, &MainUI::onUpdateBtnMenu);
    update_menu->addAction("下载升级BMS Boot", this, &MainUI::onUpdateBtnMenu);
    update_menu->addAction("下载升级BMU Boot", this, &MainUI::onUpdateBtnMenu);
    update_menu->addAction("下载升级绝缘板", this, &MainUI::onUpdateBtnMenu);
    update_menu->addAction("升级BMU", this, &MainUI::onUpdateBtnMenu);
    ui->btnVer->setMenu(update_menu);
}

void MainUI::onUpdateBtnMenu()
{
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
    return;
}
