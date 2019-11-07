#include "appinit.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include "myhelper.h"
#include "version.h"
#include "qapplication.h"
#include "qevent.h"
#include "qmutex.h"
#include "qwidget.h"
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

bool AppInit::eventFilter(QObject *obj, QEvent *evt) {
    QWidget *w = (QWidget *)obj;
    if (!w->property("canMove").toBool()) {
        return QObject::eventFilter(obj, evt);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;

    QMouseEvent *event = static_cast<QMouseEvent *>(evt);
    if (event->type() == QEvent::MouseButtonPress) {
        if (event->button() == Qt::LeftButton) {
            mousePressed = true;
            mousePoint = event->globalPos() - w->pos();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mousePressed = false;
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        if (mousePressed && (event->buttons() && Qt::LeftButton)) {
            w->move(event->globalPos() - mousePoint);
            return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

void AppInit::start() {
    // qApp->installEventFilter(this);
    sendGetRequest();
}
static int CompareVersion(QString strVer1, QString strVer2) {
    if (!strVer1.compare(strVer2)) {
        return 0;
    }

    QStringList list1 = strVer1.split(".");
    QStringList list2 = strVer2.split(".");

    int iTotal1 = list1.count();
    int iTotal2 = list2.count();
    int iTotal = iTotal1 > iTotal2 ? iTotal2 : iTotal1;

    int iValue1 = 0, iValue2 = 0;
    bool ibOK1 = false, ibOK2 = false;
    for (int iNum = 0; iNum < iTotal; ++iNum) {
        iValue1 = list1[iNum].toInt(&ibOK1);
        if (!ibOK1) {
            iValue1 = 0;
        }

        iValue2 = list2[iNum].toInt(&ibOK2);
        if (!ibOK2) {
            iValue2 = 0;
        }

        if (iValue1 == iValue2) {
            continue;
        } else if (iValue1 < iValue2) {
            return -1;
        } else {
            return 1;
        }
    }

    return iTotal1 < iTotal2 ? -1 : 1;
}

void AppInit::sendGetRequest() {
    //    qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString()
    //             << QSslSocket::sslLibraryVersionString();
    QNetworkAccessManager *m_pHttpMgr = new QNetworkAccessManager();
    //    QSslConfiguration config;
    //    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    //    config.setProtocol(QSsl::TlsV1SslV3);

    //设置url
    QString url = "http://leeginger.coding.me/autoUpdate/bms_tool.json";
    QNetworkRequest requestInfo;
    // requestInfo.setSslConfiguration(config);
    requestInfo.setUrl(QUrl(url));

    //添加事件循环机制，返回后再运行后面的
    QEventLoop eventLoop;
    QNetworkReply *reply = m_pHttpMgr->get(requestInfo);
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();  // block until finish
    //错误处理
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "request protobufHttp NoError";
    } else {
        qDebug() << "request protobufHttp handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        // statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        qDebug("request protobufHttp found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        qDebug(qPrintable(reply->errorString()));
    }
    //请求返回的结果
    QByteArray responseByte = reply->readAll();
    QJsonParseError jsonpe;
    QJsonDocument json = QJsonDocument::fromJson(responseByte, &jsonpe);
    if (jsonpe.error == QJsonParseError::NoError) {
        if (json.isObject()) {
            QJsonObject obj = json.object();
            if (obj.contains("name") && obj.contains("verison")) {
                QString name = obj["name"].toString();
                QString version = obj["verison"].toString();
                if (name == "bms_tool") {
                    int rc = CompareVersion(VER_FILEVERSION_STR, version);
                    qDebug() << rc << ":" << obj;
                    if (rc < 0) {
                        myHelper::ShowMessageBoxInfo(QString(tr("检测到新版本(%1)")).arg(version));
                    } else {
                        qDebug() << VER_FILEVERSION_STR << "===>" << version;
                    }
                }
            }
        } else {
            qDebug() << "error, shoud json object";
        }
    } else {
        qDebug() << "error:" << jsonpe.errorString();
    }
}
