#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>

#include <QFile>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QTime>
#include <QUrl>
#include <QNetworkReply>
class DownLoadManager : public QObject {
  Q_OBJECT
 public:
  explicit DownLoadManager(QObject* parent = 0);
   ~DownLoadManager();
  void setDownInto(bool isSupportBreakPoint);
  QString getDownloadUrl();
  void downloadFile(QString url, QString fileName);
  bool syncDownloadFile(QString url, QString fileName);
  // 下载进度信息;
  void stopWork();
  void stopDownload();
  void reset();
  void removeFile(QString fileName);
  void closeDownload();
 signals:
  void signalDownloadProcess(qint64, qint64);
  void signalReplyFinished(int);
  void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void signalDownloadError();

 private slots:
  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void onReadyRead(/*QString*/);
  void onFinished();
  void onError(QNetworkReply::NetworkError code);
 private:
  QNetworkAccessManager* m_networkManager;
  bool m_isSupportBreakPoint;
  int m_bytesReceived;
  int m_bytesTotal;
  int m_bytesCurrentReceived;
  bool m_isStop;
  QString m_fileName;
  QUrl m_url;
  QNetworkReply* m_reply;
};

#endif  // DOWNLOADMANAGER_H
