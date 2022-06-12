#include "main_ui.h"
#include <QTimer>
#include "FramelessHelper.h"
#include "Toast.h"
#include "cmu4u.h"
#include "iconhelper.h"
//#include "models/frmcustomplot/frmsimple.h"
#include "ui_main_ui.h"
#include "utils.h"
#include "version.h"
#include "widget.h"
#include "views/scan_settings.h"
MainUI::MainUI(QWidget* parent) : QWidget(parent), ui(new Ui::MainUI) {
    ui->setupUi(this);
    this->initForm();
    this->initLeftMain();
    this->initLeftConfig();
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
#include "stategroupbox.h"
void MainUI::initForm() {
    this->setProperty("form", true);
    //    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
    //                         Qt::WindowMinMaxButtonsHint|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf073), 40);
    IconHelper::Instance()->setIcon(ui->btnMenu, QChar(0xf00b));
    IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
    IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf067));
    IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));
#if 1  // use FramelessHelper on windows
    auto helper = new FramelessHelper(this);
    if (myHelper::level > 0 && myHelper::level != 31) {
        helper->setDisableMaximized(true);
        this->setWindowFlags(Qt::FramelessWindowHint);
    }
    helper->setDraggableMargins(3, 3, 3, 3);
    helper->setMaximizedMargins(0, 0, 0, 0);
    helper->setTitleBarHeight(32);

    helper->addExcludeItem(ui->btnMenu_Max);
    helper->addExcludeItem(ui->btnMenu_Min);
    helper->addExcludeItem(ui->btnMenu_Close);
    helper->addExcludeItem(ui->btnMenu);
    connect(ui->btnMenu_Min, &QPushButton::clicked, helper, &FramelessHelper::triggerMinimizeButtonAction);
    connect(ui->btnMenu_Max, &QPushButton::clicked, helper, &FramelessHelper::triggerMaximizeButtonAction);
    connect(ui->btnMenu_Close, &QPushButton::clicked, helper, &FramelessHelper::triggerCloseButtonAction);
#else
    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTitle->installEventFilter(this);
    this->setWidget(this);
    ui->widgetTop->setProperty("nav", "top");
#endif
    ui->labTitle->setText("库博BMS监控软件");
    ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
    this->setWindowTitle(ui->labTitle->text());
    ui->labVersion->setText(QString("battery management system v") + VER_PRODUCTVERSION_STR);

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
    ui->gridLayout_3->addWidget(new StateGroupBox());

    ui->btnMain->click();
    //创建语言切换菜单
    langue_menu = new QMenu(tr("Langue"));
    //    setChinese = new QAction(tr("Chinese"), this);
    //    setChinese->setCheckable(true);
    //    setEnglish = new QAction(tr("English"), this);
    //    setEnglish->setCheckable(true);
    //    setEnglish->setChecked(true);
    langue_menu->addAction("Chinese", this, &MainUI::menuClick);
    langue_menu->addAction("English", this, &MainUI::menuClick);
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
    title_menu->addAction("Rec转换", this, &MainUI::menuClick);
    title_menu->addAction("参数检查", this, &MainUI::menuClick);
    //    title_menu->addAction("录波转换", this, &MainUI::menuClick);
    ui->btnMenu->setMenu(title_menu);  //将主菜单设置到菜单按钮
    settings = new QSettings("config.ini", QSettings::IniFormat);
    QByteArray ba = myHelper::GetAppValue("global/layout").toByteArray();
    this->restoreGeometry(ba);
    QString user = myHelper::user;  // settings->value("global/user", "").toString();
    if (myHelper::level < 16) ui->btnMenu->hide();
    if (myHelper::level > 0 && myHelper::level != 31) {
        int index = ui->stackedWidget->addWidget(new cmu4u(this));
        ui->stackedWidget->setCurrentIndex(index);
        this->setMaximumSize(ui->stackedWidget->currentWidget()->maximumSize());
    } else if (myHelper::level == 31) {
        int index = ui->stackedWidget->addWidget(new Widget(this));
        ui->stackedWidget->setCurrentIndex(index);
        this->setMaximumSize(ui->stackedWidget->currentWidget()->maximumSize());
    }
    ui->labUser->setText(user);

    //关联换肤和切换语言功能
    ui->btnMenu->setPopupMode(QToolButton::InstantPopup);
    connect(themeGroup, &QActionGroup::triggered, this, &MainUI::changeTheme);
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

    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "系统设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "事件查询") {
        // ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "使用帮助") {
        // ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "重启") {
        qApp->exit(EXIT_CODE_REBOOT);
    }
}

void MainUI::initLeftMain() {
    pixCharMain << 0xf030 << 0xf03e << 0xf247;
    int count = btnsMain.count();
    for (int i = 0; i < count; i++) {
        btnsMain.at(i)->setCheckable(true);
        btnsMain.at(i)->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(btnsMain.at(i), SIGNAL(clicked(bool)), this, SLOT(leftMainClick()));
    }
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
void MainUI::menuClick()  //切换语言
{
    QAction* b = (QAction*)sender();
    qDebug() << b->objectName() << b->text();
    if (b->text() == "Rec转换") {
        QString path = QFileDialog::getExistingDirectory();
        FindFile(path);
        Toast::showTip("记录文件转换完毕。", nullptr);
    } else if (b->text() == "录波转换") {
        //        frmSimple* view = new frmSimple(nullptr);
        //        view->setWindowFlags(Qt::WindowCloseButtonHint);
        //        view->show();
        Toast::showTip("记录文件转换完毕。", nullptr);
    } else if (b->text() == "参数检查") {
        scan_settings* w = new scan_settings(nullptr);
        w->show();
    }
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
