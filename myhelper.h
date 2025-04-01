#ifndef MYHELPER_H
#define MYHELPER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION > QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include <QDesktopWidget>
#include <QFileDialog>
#include <QSettings>
#include "frminputbox.h"
#include "frmmessagebox.h"
#include "version.h"
#if defined(HAVE_BYTESWAP_H)
#include <byteswap.h>
#endif

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define bswap_16 OSSwapInt16
#define bswap_32 OSSwapInt32
#define bswap_64 OSSwapInt64
#endif

#if defined(__GNUC__)
#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#if GCC_VERSION >= 430
// Since GCC >= 4.30, GCC provides __builtin_bswapXX() alternatives so we switch to them
#undef bswap_32
#define bswap_32 __builtin_bswap32
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define bswap_32 _byteswap_ulong
#define bswap_16 _byteswap_ushort
#endif

#if !defined(__CYGWIN__) && !defined(bswap_16)
#warning "Fallback on C functions for bswap_16"
static inline uint16_t bswap_16(uint16_t x) { return (x >> 8) | (x << 8); }
#endif

#if !defined(bswap_32)
#warning "Fallback on C functions for bswap_32"
static inline uint32_t bswap_32(uint32_t x) { return (bswap_16(x & 0xffff) << 16) | (bswap_16(x >> 16)); }
#endif

class myHelper : public QObject {
   public:
    // 设置为开机启动
    static void AutoRunWithSystem(bool IsAutoRun, QString AppName, QString AppPath) {
        QSettings *reg = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                       QSettings::NativeFormat);

        if (IsAutoRun) {
            reg->setValue(AppName, AppPath);
        } else {
            reg->setValue(AppName, "");
        }
    }

    // 设置编码为UTF8
    static void SetUTF8Code() {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
#endif
    }

    // 设置皮肤样式
    static void SetStyle(const QString &styleName) {
        QFile file(QString(":/qss/%1.css").arg(styleName));
        file.open(QFile::ReadOnly);
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));  // 设置窗体调色板
        qApp->setStyleSheet(qss);                          // 设置主题
    }

    // 加载中文字符
    static void SetTranslation(QString local) {
        QString default_lang = "en_US";
        static QTranslator *translator = new QTranslator();
        static QTranslator *qt_translator = new QTranslator();

        Q_CHECK_PTR(translator);  // checks creation
        qApp->removeTranslator(translator);
        qApp->removeTranslator(qt_translator);

        try {
            if (translator->load(QString("%1").arg(local), ":/lang/")) {
                if (!qApp->installTranslator(translator)) {
                    qDebug("ERROR INSTALLING TRANSLATOR !!!");
                }
            } else {
                qWarning() << "ERROR LOAD TRANSLATOR !!!" << local;
                throw("ERROR LOAD TRANSLATOR");
            }
            if (qt_translator->load(QString("qt_%1.qm").arg(local), ":/lang/") && (!qt_translator->isEmpty())) {
                qDebug() << qt_translator->isEmpty();
                if (!qApp->installTranslator(qt_translator)) {
                    qWarning("ERROR INSTALLING TRANSLATOR !!!");
                }
            } else {
                qWarning() << "ERROR LOAD QT TRANSLATOR !!!" << QString("qt_") + local;
                throw("ERROR LOAD TRANSLATOR");
            }
        } catch (...) {
            if (translator->load(QString("%1").arg(default_lang), ":/lang/")) {
                if (!qApp->installTranslator(translator)) {
                }
            }
            if (qt_translator->load(QString("qt_%1.qm").arg(default_lang), ":/lang/") && (!qt_translator->isEmpty())) {
                if (!qApp->installTranslator(qt_translator)) {
                    qWarning("ERROR INSTALLING TRANSLATOR !!!");
                }
            }
        }
    }

    // 判断是否是IP地址
    static bool IsIP(QString IP) {
        QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
        return RegExp.exactMatch(IP);
    }

    // 显示输入框
    static QString showInputBox(const QString info, bool &blok) {
        frmInputBox input;

        input.setMessage(info);
        blok = input.exec();

        return input.getValue();
    }
    // 显示输入框
    static QString showInputBox(const QString info, bool &blok,const QString initText) {
        frmInputBox input;

        input.setInitText(initText);
        input.setMessage(info);
        blok = input.exec();

        return input.getValue();
    }
    // 显示信息框,仅确定按钮
    static void ShowMessageBoxInfo(QString info) {
        frmMessageBox *msg = new frmMessageBox;
        msg->SetMessage(info, 0);
        msg->exec();
    }

    // 显示错误框,仅确定按钮
    static void ShowMessageBoxError(QString info) {
        frmMessageBox *msg = new frmMessageBox;
        msg->SetMessage(info, 2);
        msg->exec();
    }

    // 显示询问框,确定和取消按钮
    static int ShowMessageBoxQuesion(QString info) {
        frmMessageBox *msg = new frmMessageBox;
        msg->SetMessage(info, 1);
        return msg->exec();
    }

    // 延时
    static void Sleep(int msec) {
        QTime dieTime = QTime::currentTime().addMSecs(msec);
        while (QTime::currentTime() < dieTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
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
    static QString IDToString(uint16_t id, uint16_t num) {
        return (num > 0) ? QString("%1-%2").arg(id / num + 1).arg(id % num + 1) : QString("%1-%2").arg(id).arg(num);
    }
    static QString IntegerToHexString(uint32_t ip) {
        return QString("%1.%2.%3.%4")
            .arg((ip >> 24) & 0xFF, 0, 10)
            .arg((ip >> 16) & 0xFF, 0, 10)
            .arg((ip >> 8)  & 0xFF, 0, 10)
            .arg((ip >> 0)  & 0xFF, 0, 10);
    }
    static void SetAppValue(const QString &key, const QVariant &value) {
        QSettings(QSettings::IniFormat, QSettings::UserScope, VER_COMPANYNAME_STR, VER_FILEDESCRIPTION_STR)
            .setValue(key, value);
    }
    static QVariant GetAppValue(const QString &key, const QVariant &defaultValue = QVariant()) {
        return QSettings(QSettings::IniFormat, QSettings::UserScope, VER_COMPANYNAME_STR, VER_FILEDESCRIPTION_STR)
            .value(key, defaultValue);
    }
};

#endif  // MYHELPER_H
