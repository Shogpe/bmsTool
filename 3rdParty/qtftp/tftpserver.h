#ifndef TFTPSERVER_H
#define TFTPSERVER_H

#include <QDir>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QtNetwork/QUdpSocket>
#define RTN_OK     0
#define RTN_FAILED -1
#define TFTP_DATALEN 512

class TFTPSenssion : public QObject {
    Q_OBJECT
   public:
    QString clientKey;
    QHostAddress clientAddr;
    quint16 clientPort;
    unsigned int requestBlockSize;
    bool tsizeReceived;
    QString requestedFilename;
    QFile requestedFile;
    qint64 requestedFileSize;
    QTimer timeoutTimer;
    unsigned int retryCount;
    // What has been sent; -1 - error packet, 0 - OACK packet, 1+ - current file packet
    int currentBlock;
    QByteArray currentBuffer;

    struct tftp_header {
        quint16 opcode;
        union {
            struct {
                quint16 block;
                char data[0];
            } data;
            char path[0];
        };
    };
    char buffer[TFTP_DATALEN + sizeof(tftp_header)];

    TFTPSenssion(QObject* parent = nullptr) {}
    ~TFTPSenssion() {
        if (requestedFile.isOpen()) {
            requestedFile.close();
        }
        if (timeoutTimer.isActive()) {
            timeoutTimer.stop();
        }
    }

};
class TFTPServer : public QObject {
    Q_OBJECT
   private:
    bool m_allowMasks;
    QString m_statusText;
    QDir m_rootDir;
    QDir m_rxDir;
    QUdpSocket m_socket;
    QMutex m_mutex;
    QThread worker;
    // If we ever support multiple simultaneous sessions, those should be there

    QHostAddress m_clientAddr;
    quint16 m_clientPort;
    //    typedef struct {
    //        QString clientKey;
    //        QHostAddress clientAddr;
    //        quint16 clientPort;
    //        unsigned int requestBlockSize;
    //        bool tsizeReceived;
    //        QString requestedFilename;
    //        QFile requestedFile;
    //        qint64 requestedFileSize;
    //        QTimer timeoutTimer;
    //        unsigned int retryCount;
    //        // What has been sent; -1 - error packet, 0 - OACK packet, 1+ - current file packet
    //        int currentBlock;
    //        QByteArray currentBuffer;
    //    } TFTPSenssion;
    QMap<QString, TFTPSenssion*> m_clients;
    enum Block : quint16 {
        B_RRQ = 1,    // read request
        B_WRQ = 2,    // write request
        B_DATA = 3,   // data packet
        B_ACK = 4,    // acknowledgement
        B_ERROR = 5,  // error code
    };
    struct tftp_header {
        quint16 opcode;
        union {
            struct {
                quint16 block;
                char data[0];
            } data;
            char path[0];
        };
    };

   private:
    void sendArray(TFTPSenssion* sess, const QByteArray& array);
    void reset(TFTPSenssion* sess);
    bool parseReadRequest(TFTPSenssion* sess, const QByteArray& array);
    bool parseWriteRequest(TFTPSenssion* sess, const QByteArray& array);
    void reportError(TFTPSenssion* sess, const QString& shortmsg, const QString& longmsg);
    void sendCurrentBlock(TFTPSenssion* sess);
    void setStatus(const QString& text);
    void sendAck(TFTPSenssion* sess,quint16 block);

   public:
    TFTPServer(QObject* parent = 0);
    ~TFTPServer();
    void startServer();
    void stopServer();
    bool init(const QString& ip, const int port, const QString& txFilepath, const QString& rxFilepath);
    QString status() const;

   signals:
    void statusUpdate(QString status);
    void infoMessage(QString message);
    void fileTransferProgress(unsigned int bytessent, unsigned int bytestotal);
    void fileTransferFinished(int ret, QString filename);

   public slots:
    void readyRead();
    //    void readyDatagram(QHostAddress* host = nullptr, quint16* port = nullptr, const QByteArray& array);
   private slots:
    void timeout(TFTPSenssion* sess);
};

#endif  // TFTPSERVER_H
