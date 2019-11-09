#ifndef APPINIT_H
#define APPINIT_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
class AppInit : public QObject
{
    Q_OBJECT
public:
    static AppInit *Instance();
    explicit AppInit(QObject *parent = 0);

    void start();
protected:

private:
    static AppInit *self;
    QNetworkAccessManager *manager;		//定义网络请求对象
    void sendGetRequest();

signals:

private slots:
    void replyFinished(QNetworkReply *reply);

};

#endif // APPINIT_H
