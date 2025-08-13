#include "logindialog.h"
#include "config.h"
#include "db_manager.h"
#include "myhelper.h"
#include "ui_logindialog.h"
#include <QDateTime>
#include <QProcess>
#include <QCryptographicHash>
#include <QSettings>


#define AUTO_FILL_DEBUG_MODE   1


#define SUPER_PW    "1a2jgPAG93tAg1V39CaH1f83c3H5"

#define GUEST_PW    "cubenergy"

#define PW_URL      "https://manage.cubenergy.com.cn/customer/#/bmsVersion"

#define PW_LEN      6
#define PW_GEN_HEAD      "cube energy "
#define PW_GEN_END       " bms tool"

//允许的误差时间数，可以允许使用的历史或更新的动态验证码个数，如果是3，就是前面3个后面3个加上自己，一共7个
#define PW_TIME_MESS_NUM    10
#define LIST_NUM    (2*PW_TIME_MESS_NUM +1)



QString getUUID() {

    // 企业版获取uuid会有问题，这里改成获取绝对路径生成一个类似的id,大概率每个人用的时候路径不一样，大概率吧
    // 如果连这都还有在出现logindialog之后闪退的问题，将 AUTO_FILL_DEBUG_MODE 宏 改为 0 ，不再允许密码填充
    static QString uuid;

    if (uuid.isEmpty()) {
        QString uniqueBase = QCoreApplication::applicationDirPath();

        if(uniqueBase.isEmpty())
        {
            //如果连这个都获取不了，只能给按天刷新的uuid了。这样保证当天不用重新输入密码
            uniqueBase = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        }

        qDebug() << "uniqueBase" << uniqueBase;

        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(uniqueBase.toUtf8());
        QByteArray result = hash.result().toHex();

        uuid = QString("%1-%2-%3-%4-%5")
            .arg(QString(result.mid(0, 8)))
            .arg(QString(result.mid(8, 4)))
            .arg(QString(result.mid(12, 4)))
            .arg(QString(result.mid(16, 4)))
            .arg(QString(result.mid(20, 12)));
    }

    return uuid;
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

    this->adjustSize();

#if AUTO_FILL_DEBUG_MODE
    QString uuid = "";

    QSettings *settings = new QSettings("config.ini", QSettings::IniFormat);
    QString uuidSettings = settings->value("pcUUID", "XXXX").toString();

    if(uuidSettings != "XXXX")
    {
        uuid = getUUID();
    }

    qDebug() << "uuid:" << uuid << uuidSettings;
    if(uuid == uuidSettings)
    {
        ui->lineEdit_pwd->setText(SUPER_PW);
    }

    delete settings;
#endif
}

// cubenergy
logindialog::~logindialog() { delete ui; }

void logindialog::on_pushButton_login_clicked() {

    int level = 0;

    if (ui->lineEdit_pwd->text() == "")
    {
        myHelper::ShowMessageBoxError(tr("动态密码不能为空!"));
        return;
    }

    if(ui->lineEdit_pwd->text() != "")
    {
        getPw();

        if(ui->lineEdit_pwd->text() == SUPER_PW)
        {
            level = db_manager::LEVEL_DEBUG;
            db_manager::Instance()->setUserLevel(level);

            QString uuid = getUUID();
            QSettings *settings = new QSettings("config.ini", QSettings::IniFormat);
            settings->setValue("pcUUID",uuid);

            accept();
        }
        else if(ui->lineEdit_pwd->text() == GUEST_PW)
        {
            level = db_manager::LEVEL_GUEST;
            db_manager::Instance()->setUserLevel(level);
            accept();
        }
        else
        {
            foreach(QString s, pwList)
            {
                if(ui->lineEdit_pwd->text().simplified() == s)
                {
                    level = db_manager::LEVEL_SUPER;
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



void logindialog::on_lineEdit_pwd_returnPressed()
{
    ui->pushButton_login->click();
}

