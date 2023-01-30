#include "main_ui.h"
#include <QTimer>
#include "FramelessHelper.h"
#include "Toast.h"
//#include "cmu4u.h"
#include "iconhelper.h"
//#include "models/frmcustomplot/frmsimple.h"
#include "bmsview.h"
#include "cmu_ip.h"
#include "scan_settings.h"
#include "ui_main_ui.h"
#include "utils.h"
#include "version.h"
//#include "rtu_tool.h"
#include "bms_datalog.h"
#include "firmwareDialog.h"
void MainUI::closeEvent(QCloseEvent* event) {
    //判断账号输入框是否为空（只是作为一个条件）
    if (myHelper::ShowMessageBoxQuesion(tr("确定要关闭本程序吗？")) == QDialog::Accepted) {
        //接收这个事件,当前窗口会关闭
        event->accept();
    } else {
        //忽略这个事件，当前窗口不会关闭
        event->ignore();
    }
}
MainUI::MainUI(QWidget* parent) : QWidget(parent), ui(new Ui::MainUI) {
    ui->setupUi(this);
    this->initForm();

    this->tftpd = new TFTPServer();
    // 可建立全局实例
    manager = new NotifyManager(this);
    // 可选修改默认参数
    manager->setMaxCount(5);             // 最大显示消息数，默认5
    manager->setDisplayTime(5000);       // 显示时间，默认10000（毫秒）
    manager->setAnimateTime(500);        // 动画时间，默认300（毫秒）
    manager->setSpacing(5);              // 消息框间距，默认10px
    manager->setCornerMargins(20, 20);   // 右下角边距，默认10, 10
    manager->setNotifyWndSize(300, 75);  // 消息框大小，默认300, 60
    //    manager->setDefaultIcon(":/image/message.png");  // 消息图标，默认":/message.png"
    manager->setShowQueueCount(true);  // 是否显示超出最大数未显示的消息数量，默认true
    //    manager->setStyleSheet("#notify-background {....", "自定义主题名称"); //
    //    添加自定义主题样式表，默认样式主题名为default
    // 基本用法
    //    manager->notify("消息标题", "消息主体");
    connect(tftpd, &TFTPServer::statusUpdate, this,
            [this](QString status) { ui->lTftpStatus->setText(QString("%1:%2").arg(tr("升级服务"), status)); });

    connect(tftpd, &TFTPServer::fileTransferFinished, this, [this](int ret, QString msg) {
        manager->notify("TFTP", QString("%1:%2").arg(msg, ret == 0 ? tr("成功") : tr("失败")));
    });
    tftpd->init("192.168.1.230", 69, "firmware");
}

MainUI::~MainUI() {
    if (timer) timer->deleteLater();
    QByteArray ba = this->saveGeometry();
    myHelper::SetAppValue("global/layout", ba);
    settings->sync();
    settings->deleteLater();
    delete settings;
    delete ui;
}
void MainUI::initForm() {
    this->setWindowFlags(Qt::FramelessWindowHint);
    IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf073), 40);
    IconHelper::Instance()->setIcon(ui->btnMenu, QChar(0xf00b));
    IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
    IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf2d0));
    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));
#if 1  // use FramelessHelper on windows
    auto helper = new FramelessHelper(this);
    if (db_manager::Instance()->userLevel() > 0 && db_manager::Instance()->userLevel() != 31) {
        helper->setDisableMaximized(true);
        this->setWindowFlags(Qt::FramelessWindowHint);
    }
    helper->setDraggableMargins(3, 3, 3, 3);
    helper->setMaximizedMargins(3, 3, 3, 3);
    helper->setTitleBarHeight(ui->titleBar->sizeHint().height());

    helper->addExcludeItem(ui->btnMenu_Max);
    helper->addExcludeItem(ui->btnMenu_Min);
    helper->addExcludeItem(ui->btnMenu_Close);
    helper->addExcludeItem(ui->btnMenu);
    connect(ui->btnMenu_Min, &QPushButton::clicked, helper, &FramelessHelper::triggerMinimizeButtonAction);
    connect(ui->btnMenu_Max, &QPushButton::clicked, helper, &FramelessHelper::triggerMaximizeButtonAction);
    connect(ui->btnMenu_Close, &QPushButton::clicked, helper, &FramelessHelper::triggerCloseButtonAction);
    connect(helper, &FramelessHelper::maximizedChanged, this, [this](bool max) {
        if (max) {
            IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf2d2));
            ui->btnMenu_Max->setToolTip(tr("Restore"));
        } else {
            IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf2d0));
            ui->btnMenu_Max->setToolTip(tr("Maximize"));
        }
        ui->btnMenu_Max->setAttribute(Qt::WA_UnderMouse, false);
    });
#else
    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTitle->installEventFilter(this);
    this->setWidget(this);
    ui->widgetTop->setProperty("nav", "top");
#endif
    ui->labTitle->setText(tr("库博BMS监控软件"));
    ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
    this->setWindowTitle(ui->labTitle->text());
    ui->labVersion->setText(QString("battery management system v") + VER_PRODUCTVERSION_STR);
    // 测试版本提示
    const int time_tip = 31 * 24 * 60 * 60;
    //    const int time_tip = 0;
    if (time(nullptr) > (myHelper::cvt_TIME(__DATE__) + time_tip) &&
        (QString(VER_PRODUCTVERSION_STR).contains(QRegExp("[a-zA-Z]")))) {
        qDebug() << "timeout exit..";
        myHelper::ShowMessageBoxError(tr("本软件为测试使用，请勿长时间使用!"));
        if (time(nullptr) > (myHelper::cvt_TIME(__DATE__) + (2 * 31 * 24 * 60 * 60))) {
            exit(0);
        }
    }
    QSize icoSize(32, 32);
    int icoWidth = 85;

    //设置顶部导航按钮
    QList<QToolButton*> tbtns = ui->widgetTop->findChildren<QToolButton*>();
    foreach (QToolButton* btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
        btn->hide();
    }
    ui->widgetTop->hide();
    //
    ui->btnMain->click();
    //创建语言切换菜单
    QMenu* langue_menu = new QMenu(tr("Langue"), this);
    QString locale = myHelper::GetAppValue("locale", "zh_CN").toString();

    langue_menu->addAction("简体中文", this, &MainUI::menuClick);
    langue_menu->actions().constLast()->setObjectName("zh_CN");
    langue_menu->addAction("English", this, &MainUI::menuClick);
    langue_menu->actions().constLast()->setObjectName("en_US");
    langue_menu->addAction("繁體中文", this, &MainUI::menuClick);
    langue_menu->actions().constLast()->setObjectName("zh_TW");
    QActionGroup* langueGroup = new QActionGroup(this);
    foreach (QAction* act, langue_menu->actions()) {
        langueGroup->addAction(act);
        act->setCheckable(true);
        if (locale == act->objectName()) act->setChecked(true);
    }

    //创建主菜单,将主题和语言菜单当二级菜单加入主菜单
    QMenu* title_menu = new QMenu(this);
    title_menu->addMenu(langue_menu);
    //    title_menu->addMenu(theme_menu);
    title_menu->addAction(tr("Rec转换"), this, &MainUI::menuClick);
    title_menu->actions().constLast()->setObjectName("Rec Convert");
    title_menu->addAction(tr("维护工具"), this, &MainUI::menuClick);
    title_menu->actions().constLast()->setObjectName("Maintenance Tool");

    if (QFileInfo("User Manual.pdf").isFile()) {
        title_menu->addAction(tr("用户手册"), this, &MainUI::menuClick);
        title_menu->actions().constLast()->setObjectName("User Manual");
    }
    if (db_manager::Instance()->userName() == "Ganing") {
        title_menu->addAction(tr("故障录波解析"), this, &MainUI::menuClick);
        title_menu->actions().constLast()->setObjectName("DataLog");
        title_menu->addAction(tr("新增BMS页面"), this, &MainUI::menuClick);
        title_menu->actions().constLast()->setObjectName("BmsView");
    }
    //    title_menu->addAction("固件查看", this, &MainUI::menuClick);
    //    title_menu->addAction("录波转换", this, &MainUI::menuClick);
    ui->btnMenu->setMenu(title_menu);  //将主菜单设置到菜单按钮
    settings = new QSettings("config.ini", QSettings::IniFormat);
    QByteArray ba = myHelper::GetAppValue("global/layout").toByteArray();
    this->restoreGeometry(ba);
    QString user = db_manager::Instance()->userName();  // settings->value("global/user", "").toString();
    if (db_manager::Instance()->userLevel() < 16) ui->btnMenu->hide();
    if (db_manager::Instance()->userLevel() > 0 && db_manager::Instance()->userLevel() != 31) {
        int index = ui->stackedWidget->addWidget(new BMSView(this));
        ui->stackedWidget->setCurrentIndex(index);
        this->setMaximumSize(ui->stackedWidget->currentWidget()->maximumSize());
    } else if (db_manager::Instance()->userLevel() == 1) {
        int index = ui->stackedWidget->addWidget(new CmuIpView(this));
        ui->stackedWidget->setCurrentIndex(index);
        this->setMaximumSize(ui->stackedWidget->currentWidget()->maximumSize());
    } else if (db_manager::Instance()->userLevel() == 31) {  // Widget,RTUView
        int index = ui->stackedWidget->addWidget(new BMSView(this));
        ui->stackedWidget->setCurrentIndex(index);
        //        this->setMaximumSize(ui->stackedWidget->currentWidget()->maximumSize());
    }
    ui->labUser->setText(user);
    //关联换肤和切换语言功能
    ui->btnMenu->setPopupMode(QToolButton::InstantPopup);
    //    connect(themeGroup, &QActionGroup::triggered, this, &MainUI::changeTheme);
    this->timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this,
            [=]() { ui->labTime->setText(QDateTime::currentDateTime().toString("hh:mm:ss")); });
    timer->start(500);
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

    //    if (name == "主界面") {
    //        ui->stackedWidget->setCurrentIndex(0);
    //    } else if (name == "系统设置") {
    //        ui->stackedWidget->setCurrentIndex(1);
    //    } else if (name == "事件查询") {
    //        // ui->stackedWidget->setCurrentIndex(2);
    //    } else if (name == "使用帮助") {
    //        // ui->stackedWidget->setCurrentIndex(3);
    //    } else if (name == "重启") {
    //        qApp->exit(EXIT_CODE_REBOOT);
    //    }
}

void MainUI::menuClick()  //切换语言
{
    QAction* b = (QAction*)sender();
    qDebug() << b->objectName() << b->text();
    if (b->objectName() == "Rec Convert") {
        QString path = QFileDialog::getExistingDirectory();
        FindFile(path);
    } else if (b->objectName() == "DataLog") {
        BmsDataLog* view = new BmsDataLog(nullptr);
        view->log2csv();
        view->deleteLater();
    } else if (b->objectName() == "BmsView") {
        BMSView* view = new BMSView(nullptr);
        view->show();
    } else if (b->objectName() == "Maintenance Tool") {
        scan_settings* w = new scan_settings(this);
        w->show();
        //    } else if (b->text() == "固件查看") {
        //        firmwareDialog* w = new firmwareDialog(nullptr);
        //        w->show();
    } else if (b->objectName() == "en_US" || b->objectName() == "zh_CN" || b->objectName() == "zh_TW") {
        qDebug() << "set eng";
        myHelper::SetAppValue("Locale", b->objectName());
        myHelper::SetTranslation(b->objectName());
    } else if (b->objectName() == "User Manual") {
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile("User Manual.pdf"))) {
            myHelper::ShowMessageBoxError("open User Manual docment failed!Please install pdf reader.");
        }
    }
}
