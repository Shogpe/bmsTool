#ifndef MYHELPER_H
#define MYHELPER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION > QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include <QDesktopWidget>
#include "frmmessagebox.h"

class myHelper : public QObject {
   public:
    //设置为开机启动
    static void AutoRunWithSystem(bool IsAutoRun, QString AppName, QString AppPath) {
        QSettings *reg = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                       QSettings::NativeFormat);

        if (IsAutoRun) {
            reg->setValue(AppName, AppPath);
        } else {
            reg->setValue(AppName, "");
        }
    }

    //设置编码为UTF8
    static void SetUTF8Code() {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
#endif
    }

    //设置皮肤样式
    static void SetStyle(const QString &styleName) {
        QFile file(QString(":/qss/%1.css").arg(styleName));
        file.open(QFile::ReadOnly);
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));  //设置窗体调色板
        qApp->setStyleSheet(qss);                          //设置主题
    }

    //加载中文字符
    static void SetChinese() {
        QTranslator *translator = new QTranslator(qApp);
        translator->load(":/lang/zh_CN.qm");
        qApp->installTranslator(translator);
    }

    //判断是否是IP地址
    static bool IsIP(QString IP) {
        QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
        return RegExp.exactMatch(IP);
    }

    //显示信息框,仅确定按钮
    static void ShowMessageBoxInfo(QString info) {
        frmMessageBox *msg = new frmMessageBox;
        msg->SetMessage(info, 0);
        msg->exec();
    }

    //显示错误框,仅确定按钮
    static void ShowMessageBoxError(QString info) {
        frmMessageBox *msg = new frmMessageBox;
        msg->SetMessage(info, 2);
        msg->exec();
    }

    //显示询问框,确定和取消按钮
    static int ShowMessageBoxQuesion(QString info) {
        frmMessageBox *msg = new frmMessageBox;
        msg->SetMessage(info, 1);
        return msg->exec();
    }

    //延时
    static void Sleep(int sec) {
        QTime dieTime = QTime::currentTime().addMSecs(sec);
        while (QTime::currentTime() < dieTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }

    //窗体居中显示
    static void FormInCenter(QWidget *frm) {
        int frmX = frm->width();
        int frmY = frm->height();
        QDesktopWidget w;
        int deskWidth = w.width();
        int deskHeight = w.height();
        QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeight / 2 - frmY / 2);
        frm->move(movePoint);
    }
    static time_t cvt_TIME(char const *Date) {
        char s_month[5];
        int month, day, year;
        struct tm t;
        memset(&t, 0, sizeof(t));
        static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
        sscanf(Date, "%3s %d %d", s_month, &day, &year);
        month = (strstr(month_names, s_month) - month_names) / 3;
        t.tm_mon = month;
        t.tm_mday = day;
        t.tm_year = year - 1900;
        t.tm_isdst = -1;
        return mktime(&t);
    }
    static uint32_t IPV4StringToInteger(const QString &ip) {
        QStringList ips = ip.split(".");
        if (ips.size() == 4) {
            return ips.at(3).toULong() | ips.at(2).toULong() << 8 | ips.at(1).toULong() << 16 |
                   ips.at(0).toULong() << 24;
        }
        return 0;
    }
    static QString IPV4IntegerToString(uint32_t ip) {
      return QString("%1.%2.%3.%4")
          .arg((ip >> 24) & 0xFF)
          .arg((ip >> 16) & 0xFF)
          .arg((ip >> 8) & 0xFF)
          .arg(ip & 0xFF);
    }
    static QString IntegerToHexString(uint32_t ip) {
      return QString("%1.%2.%3.%4")
          .arg((ip >> 24) & 0xFF,0,16)
          .arg((ip >> 16) & 0xFF,0,16)
          .arg((ip >> 8) & 0xFF,0,16)
          .arg(ip & 0xFF,0,16);
    }
};

#endif  // MYHELPER_H
