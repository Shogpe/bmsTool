#include "appinit.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include "QBreakpadHandler.h"
#include "downloadmanager.h"
#include "myhelper.h"
#include "version.h"
const QString url1 = "https://gitee.com/lganing/demo/raw/master/uploads/bms_tool.json";
const QString url1_db = "https://gitee.com/lganing/demo/raw/master/uploads/bms_tool.db";
const QString url2 = "https://leeginger.coding.net/p/autoUpdate/d/autoUpdate/git/raw/master/bms_tool.json";

AppInit *AppInit::self = nullptr;
AppInit *AppInit::Instance() {
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new AppInit;
        }
    }

    return self;
}

AppInit::AppInit(QObject *parent) : QObject(parent) {}

void AppInit::start() {
    QBreakpadInstance.setDumpPath(".");
    myHelper::SetStyle("lightblue");
    //    DownLoadManager *m_download = new DownLoadManager();
    //    if (!m_download->syncDownloadFile(url1, "bms_tool.json")) {
    //        return;
    //    }
    //    m_download->deleteLater();

    QLocale local = QLocale::system();
    QString default_locale = local.name();
    QString setLocale = myHelper::GetAppValue("locale", default_locale).toString();
    qDebug() << setLocale;
    myHelper::SetTranslation(setLocale);
    updateCheck(url1);
}
static int CompareVersion(QString curVer, QString chkVer) {
    if (!curVer.compare(chkVer)) {
        return 0;
    }
    int rc = QVersionNumber::compare(QVersionNumber::fromString(curVer), QVersionNumber::fromString(chkVer));
    return rc;
}

void AppInit::replyFinished(QNetworkReply *reply)  //当回复结束后
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "upgrade Error";
        // 请求错误时二次检查
        if (reply->request().url().toString() == url2) return;
        updateCheck(url2);
    }
    //请求返回的结果
    QByteArray responseByte = reply->readAll();
    reply->deleteLater();  //最后要释放reply对象

    QJsonParseError jsonpe;
    QJsonDocument json = QJsonDocument::fromJson(responseByte, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError) {
        if (json.isObject()) {
            QJsonObject obj = json.object();
            if (obj.contains("name") && obj.contains("verison") && obj.contains("url") && obj.contains("date") &&
                obj.contains("desc")) {
                QString name = obj["name"].toString();
                int type = obj["type"].toInt();
                QString version = obj["verison"].toString();
                QString url = obj["url"].toString();
                QString date = obj["date"].toString();
                QString desc = obj["desc"].toString();
                if (type == 1) {
                    url = QString(QByteArray::fromBase64((url + "=").toUtf8()));
                }
                qWarning() << VER_FILEVERSION_STR << "===>" << version << CompareVersion(VER_FILEVERSION_STR, version);
                if (CompareVersion(VER_FILEVERSION_STR, version) < 0) {
                    QMessageBox box;
                    QString warningStr =
                        "检测到新版本!\n版本号：" + version + "\n" + "更新时间：" + date + "\n" + "更新说明：" + desc;
                    int ret = box.warning(nullptr, "检查更新", warningStr, "去下载", "不更新");
                    if (ret == 0)  //点击更新
                    {
                        QDesktopServices::openUrl(QUrl(url));
                    }
                }
                //                if (obj.contains("db_version")) {
                //                    QString db_version = obj["db_version"].toString();
                //                    QString db_file_ver = "0.0.0.0";
                //                    QString dbfileName = "data.db3";
                //                    QFileInfo info(dbfileName);
                //                    if (info.exists()) {
                //                        db_file_ver = info.lastModified().toString("yyyy.MM.dd.hh");
                //                    }
                //                    if (CompareVersion(db_file_ver, db_version) < 0) {
                //                        DownLoadManager *m_download = new DownLoadManager();
                //                        if (!m_download->syncDownloadFile(url1_db, dbfileName)) {
                //                            //                            return;
                //                        }
                //                        m_download->deleteLater();
                //                    }
                //                }
            }

        } else {
            qDebug() << "error, shoud json object";
        }
    } else {
        qDebug() << "error:" << jsonpe.errorString();
    }
}

void AppInit::updateCheck(QString url) {
    qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString()
             << QSslSocket::sslLibraryVersionString();
    QSslConfiguration config;
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1SslV3);
    QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
    //设置url
    QNetworkRequest requestInfo;
    requestInfo.setSslConfiguration(config);
    requestInfo.setUrl(QUrl(url));
    //添加事件循环机制，返回后再运行后面的
    accessManager->get(requestInfo);
    connect(accessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    //错误处理
}
