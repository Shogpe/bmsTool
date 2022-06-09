#include "logindialog.h"
#include "config.h"
#include "db_manager.h"
#include "myhelper.h"
#include "ui_logindialog.h"
QString myHelper::user = "";
int myHelper::level = 0;
// 加解密都用此方法
QByteArray toXOREncryptUncrypt(QByteArray src, const QChar key) {
    for (int i = 0; i < src.count(); i++) {
        src[i] = src.at(i) ^ key.toLatin1();
    }
    return src;
}
logindialog::logindialog(QWidget *parent) : QDialog(parent), ui(new Ui::logindialog) {
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint);
    //在构造函数里将密码框的显示设置为黑点，不可见
    ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);
    QString qstrname = myHelper::GetAppValue("user/name", "").toString();
    QString qstrpasswd = myHelper::GetAppValue("user/password", "").toString();
    bool isRemenber = myHelper::GetAppValue("user/remenber", false).toBool();
    ui->isRemember->setChecked(isRemenber);
    if (qstrpasswd.length() > 0) {
        QByteArray ba = QByteArray::fromBase64(qstrpasswd.toLocal8Bit());
        qstrpasswd = toXOREncryptUncrypt(ba, '$');
        ui->lineEdit_uname->setText(qstrname);
        ui->lineEdit_pwd->setText(qstrpasswd);
    }
    this->adjustSize();
}
// cubenergy
logindialog::~logindialog() { delete ui; }

void logindialog::on_pushButton_login_clicked() {
    if (ui->lineEdit_uname->text() == "" || ui->lineEdit_pwd->text() == "") {
        myHelper::ShowMessageBoxError(tr("用户名或密码不能为空!"));
        ui->lineEdit_uname->setFocus();
        return;
    }
    QString pwd = ui->lineEdit_pwd->text().append(ui->lineEdit_uname->text());
    pwd.append(ui->lineEdit_pwd->text());
    pwd = QString(QCryptographicHash::hash(pwd.toLocal8Bit(), QCryptographicHash::Md5).toHex());
//    qDebug() << pwd;
    int level = 0;
    if (db_manager::Instance()->getUser(ui->lineEdit_uname->text(), pwd, level)) {
        myHelper::user = ui->lineEdit_uname->text();
        myHelper::level = level;
        qDebug() << level;
        accept();
        if (ui->isRemember->isChecked()) {
            // 保存密码
            QByteArray ba = toXOREncryptUncrypt(ui->lineEdit_pwd->text().toLocal8Bit(), '$');
            myHelper::SetAppValue("user/name", ui->lineEdit_uname->text());
            myHelper::SetAppValue("user/password", ba.toBase64());
        }
        myHelper::SetAppValue("user/remenber", ui->isRemember->isChecked());
    } else {
        //如果不正确，弹出警告框
        myHelper::ShowMessageBoxError(tr("用户名或密码错误!"));
        //如果还想清空用户名、密码框，并且光标自动跳转到用户名输入框，就继续下面
        ui->lineEdit_pwd->clear();
        ui->lineEdit_uname->setFocus();  //将光标移到用户名框内
    }
}

void logindialog::on_pushButton_exit_clicked() { close(); }

void logindialog::on_isRemember_stateChanged(int arg1) {
    Q_UNUSED(arg1)
    if (!ui->isRemember->isChecked()) {
        myHelper::SetAppValue("user/name", "");
        myHelper::SetAppValue("user/password", "");
        myHelper::SetAppValue("user/remenber", ui->isRemember->isChecked());
    }
}
