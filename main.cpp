#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QTextCodec>
#include "appinit.h"
#include "logindialog.h"
#include "main_ui.h"
void reboot() {
    QString program = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    QString workingDirectory = QDir::currentPath();
    QProcess::startDetached(program, arguments, workingDirectory);
    QApplication::exit();
}

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);  // Icons are *no longer shown* in menus
    QApplication a(argc, argv);

#if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif
    //加载样式表
    a.setFont(QFont("Microsoft Yahei", 9));
    AppInit::Instance()->start();
    logindialog *dlg = new logindialog();
    if (dlg->exec() != QDialog::Accepted) {
        return -1;
    }
    dlg->deleteLater();
    MainUI w;
    w.show();
    int ret = a.exec();
    if (ret == EXIT_CODE_REBOOT) {
        reboot();
        return 0;
    }

    return ret;
}
