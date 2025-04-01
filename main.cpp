#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QTextCodec>
#include "appinit.h"
#include "db_manager.h"
#include "firmwareDialog.h"
#include "logindialog.h"
#include "logmanager.h"
#include "main_ui.h"
#include "scan_settings.h"
void reboot() {
    QString program = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    QString workingDirectory = QDir::currentPath();
    QProcess::startDetached(program, arguments, workingDirectory);
    QApplication::exit();
}

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
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

//    LogManager::instance()->debug_log_console_on();
    AppInit::Instance()->start();
    logindialog *dlg = new logindialog();
    if (dlg->exec() != QDialog::Accepted) {
        return -1;
    }
    dlg->deleteLater();
    MainUI w;
    //    scan_settings w;
    QProcess process(&w);
    QString toolPath = QCoreApplication::applicationDirPath()+"/toolUpdate.exe";
            process.startDetached(toolPath);
    w.show();
    int ret = a.exec();
    if (ret == EXIT_CODE_REBOOT) {
        reboot();
        return 0;
    }

    return ret;
}
