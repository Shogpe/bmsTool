#include "logindialog.h"
#include "config.h"
#include "db_manager.h"
#include "myhelper.h"
#include "ui_logindialog.h"
#include <QDateTime>

#define PW_DEBUG 0
#if PW_DEBUG
#define SUPER_PW    "1234"
#else
#define SUPER_PW    "1a2jgPAG93tAg1V39CaH1f83c3H5"
#endif

#define PW_URL      "https://manage.cubenergy.com.cn/customer/#/bmsVersion"

#define PW_LEN      6
#define PW_GEN_HEAD      "cube energy "
#define PW_GEN_END       " bms tool"

//允许的误差时间数，可以允许使用的历史或更新的动态验证码个数，如果是3，就是前面3个后面3个加上自己，一共7个
#define PW_TIME_MESS_NUM    10
#define LIST_NUM    (2*PW_TIME_MESS_NUM +1)

// 加解密都用此方法
QByteArray toXOREncryptUncrypt(QByteArray src, const QChar key) {
    for (int i = 0; i < src.count(); i++) {
        src[i] = src.at(i) ^ key.toLatin1();
    }
    return src;
}

void logindialog::getPw()
{
    pwList.clear();
    QDateTime dateTime = QDateTime::currentDateTimeUtc();

    for(int idx = -PW_TIME_MESS_NUM; idx <= PW_TIME_MESS_NUM; idx++)
    {
        QDateTime timeTemp = dateTime.addSecs(60*idx);
        QString str = timeTemp .toString("yyyy-MM-dd hh:mm");

        str = PW_GEN_HEAD + str;
        str = str + PW_GEN_END;

        QByteArray byteArray = QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5);

        QString pw = "";

        for(int idx = 0; idx < PW_LEN; idx++)
        {
            pw += QString::number(uint(byteArray[idx])%10);
        }
        pwList.append(pw);
    }
}

logindialog::logindialog(QWidget *parent) : QDialog(parent), ui(new Ui::logindialog) {
    ui->setupUi(this);
    setWindowTitle(QString("%1 %2").arg(tr("BMS 上位机"),tr("系统登录")));
    setWindowFlags(Qt::WindowCloseButtonHint);

    this->setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());

    this->activateWindow();
    QString qstrpasswd = myHelper::GetAppValue("user/password", "").toString();
//    if (qstrpasswd.length() > 0) {
//        QByteArray ba = QByteArray::fromBase64(qstrpasswd.toLocal8Bit());
//        qstrpasswd = toXOREncryptUncrypt(ba, '$');
//        ui->lineEdit_pwd->setText(qstrpasswd);
//    }
    this->adjustSize();
}

// cubenergy
logindialog::~logindialog() { delete ui; }

void logindialog::on_pushButton_login_clicked() {

    int level = 0;

    //访客模式
    if(ui->chk_guest->isChecked() == true)
    {
        level = 1;
        db_manager::Instance()->setUserLevel(level);
        accept();
    }

    if (ui->chk_guest->isChecked() == false && ui->lineEdit_pwd->text() == "")
    {
        myHelper::ShowMessageBoxError(tr("动态密码不能为空!"));
        return;
    }
    QString pwd = ui->lineEdit_pwd->text();
    pwd = QString(QCryptographicHash::hash(pwd.toLocal8Bit(), QCryptographicHash::Md5).toHex());

    if(ui->chk_guest->isChecked() == false && ui->lineEdit_pwd->text() != "")
    {
        getPw();

//        QString str = "";

//        foreach(QString s, pwList)
//        {
//            str+=s;
//            str+="\n";
//        }
//        ui->test->setText(str);

        if(ui->lineEdit_pwd->text() == SUPER_PW)
        {
            level = 3;
            db_manager::Instance()->setUserLevel(level);
            accept();
        }
        else
        {
            foreach(QString s, pwList)
            {
                if(ui->lineEdit_pwd->text() == s)
                {
                    level = 2;
                    db_manager::Instance()->setUserLevel(level);
                    accept();
                    return;
                }
            }


            //如果不正确，弹出警告框
            myHelper::ShowMessageBoxError(tr("动态验证码错误!"));
            //如果还想清空用户名、密码框，并且光标自动跳转到用户名输入框，就继续下面
            ui->lineEdit_pwd->clear();
            ui->lineEdit_pwd->setFocus();  //将光标移到用户名框内
        }
    }
}

void logindialog::on_pushButton_exit_clicked() { close(); }

void logindialog::on_chk_guest_stateChanged(int arg1)
{
    if(ui->chk_guest->isChecked())
    {
        ui->lineEdit_pwd->setEnabled(false);
    }
    else
    {
        ui->lineEdit_pwd->setEnabled(true);
    }
}


void logindialog::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl(PW_URL));
    QString tips = tr("如果没有网络，请复制下方链接用手机打开，或使用访客模式登陆\n");
    tips += PW_URL;
    tips += tr("\n在BMS版本一栏获取6位动态验证码");
    ui->lb_Tips->setText(tips);
}


void logindialog::on_lineEdit_pwd_returnPressed()
{
    ui->pushButton_login->click();
}

