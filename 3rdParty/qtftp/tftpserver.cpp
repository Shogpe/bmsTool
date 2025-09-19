#include "tftpserver.h"
#include <string.h>
#include <QtEndian>
#include "Qtftp.h"
//#define DUMP_TRAFFIC

// TFTP opcodes
static const unsigned char TFTP_OP_RRQ = 1;   //请求报文
static const unsigned char TFTP_OP_WRQ = 2;   //写请求报文
static const unsigned char TFTP_OP_DATA = 3;  //数据报文
static const unsigned char TFTP_OP_ACK = 4;   //响应报文
static const unsigned char TFTP_OP_ERR = 5;   //出错报文
static const unsigned char TFTP_OP_OACK = 6;

// TFTP error codes
static const unsigned char TFTP_ERROR_UNKNOWN = 0;

// Other parameters
static const unsigned int TFTP_TIMEOUT = 5000;  // ms
static const unsigned int TFTP_RETRIES = 3;

// https://tools.ietf.org/html/rfc1350
TFTPServer::TFTPServer(QObject* parent) : QObject(parent), m_socket(this) {
    m_statusText = "Not started";
    m_allowMasks = false;
}

TFTPServer::~TFTPServer()
{
    stopServer();
}

void TFTPServer::setStatus(const QString& text) {
    m_statusText = text;
    emit statusUpdate(m_statusText);
}

bool TFTPServer::init(const QString& ip, const int port, const QString& txFilepath, const QString& rxFilepath) {
    stopServer();
    // Set the root directory
    m_rootDir.setPath(txFilepath);
    if (!m_rootDir.exists() || !m_rootDir.isReadable()) {
        qDebug() << ("Failed: invalid root path");
        if (!m_rootDir.mkpath(".")) {
            qDebug() << QString("Failed: create %1 ").arg(txFilepath);
            setStatus("No valid path.");
            return false;
        }
    }

    // Set the root directory
    m_rxDir.setPath(rxFilepath);
    if (!m_rxDir.exists() || !m_rxDir.isReadable()) {
        qDebug() << ("Failed: invalid rx path");
        if (!m_rxDir.mkpath(".")) {
            qDebug() << QString("Failed: create %1 ").arg(rxFilepath);
            setStatus("No valid path.");
            return false;
        }
    }

    // Socket
    QHostAddress haddr;
    if (!haddr.setAddress(ip)) {
        qDebug() << ("Failed: invalid listen address");
        setStatus("host addr not avaliable.");
        return false;
    }

    if (!m_socket.bind(haddr, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qDebug() << (tr("Failed: cannot listen at port %1: %2").arg(port).arg(m_socket.errorString()));
        setStatus("host port not avaliable.");
        return false;
    }
    m_socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    startServer();
    setStatus("Running.");
    qDebug() << (tr("Running: listening at port %1").arg(port));
    return true;
}
void TFTPServer::startServer() {
    moveToThread(&worker);
    connect(&m_socket, &QUdpSocket::readyRead, this, &TFTPServer::readyRead);
//    connect(&worker, &QThread::finished, &m_socket, &QUdpSocket::deleteLater);//这里不应该用槽去删除，父对象析构的时候删除就好了，不然会删除两遍
    worker.start();
}

void TFTPServer::stopServer() {
    disconnect(&m_socket, &QUdpSocket::readyRead, this, &TFTPServer::readyRead);
    for (TFTPSenssion* sess : m_clients.values()) {
        reset(sess);
    }
    worker.quit();
    worker.wait();
    m_socket.close();
    setStatus("Stopped.");
}

//
void TFTPServer::reset(TFTPSenssion* sess) {
    sess->requestedFilename.clear();
    sess->tsizeReceived = false;
    sess->requestBlockSize = TFTP_DATALEN;
    if (sess->requestedFile.isOpen()) sess->requestedFile.close();
    sess->currentBlock = -1;
    sess->retryCount = 0;
    sess->timeoutTimer.stop();
    disconnect(&sess->timeoutTimer, &QTimer::timeout, this, nullptr);
    m_mutex.lock();
    m_clients.remove(sess->clientKey);
    m_mutex.unlock();
    sess->deleteLater();
}

void TFTPServer::reportError(TFTPSenssion* sess, const QString& shortmsg, const QString& longmsg) {
    emit fileTransferFinished(RTN_FAILED, QString("[%1]%2").arg(sess->clientKey, sess->requestedFilename));
    qDebug() << QString("[%1]fileTransferFinished FAILED,%2").arg(sess->clientKey, longmsg);
    emit infoMessage(longmsg);
    setStatus(tr("Running: [%1]%2").arg(sess->clientKey, shortmsg));
    qDebug() << sess->clientKey << longmsg;
    // Create the error message: RFC 1350
    //
    // 2 bytes     2 bytes      string    1 byte
    // -----------------------------------------
    // | Opcode |  ErrorCode |   ErrMsg   |   0  |
    //
    const char* errstr = shortmsg.toStdString().c_str();
    QByteArray array(5 + strlen(errstr), 0);
    array.data()[1] = TFTP_OP_ERR;
    array.data()[3] = TFTP_ERROR_UNKNOWN;
    strncpy(array.data() + 4, errstr, strlen(errstr));
    sess->currentBlock = -1;

    sendArray(sess, array);
}

void TFTPServer::readyRead() {
    QByteArray array;

    // Read all the available datagrams
    while (m_socket.hasPendingDatagrams()) {
        array.clear();
        array.resize(m_socket.pendingDatagramSize());
        m_socket.readDatagram(array.data(), array.size(), &m_clientAddr, &m_clientPort);
        if (array.isEmpty()) continue;
        //处理接收报文
        QString client_key = QString("%1:%2").arg(m_clientAddr.toString()).arg(m_clientPort);
#if 1//defined(DUMP_TRAFFIC)
        qDebug() << ("TFTP server: data read:", array.toHex());
#endif
        TFTPSenssion* sess = m_clients.value(client_key);
        if (!sess) {
            // new client
            sess = new TFTPSenssion;
            sess->clientKey = client_key;
            sess->clientAddr = m_clientAddr;
            sess->clientPort = m_clientPort;
            sess->currentBlock = -1;
            sess->requestBlockSize = TFTP_DATALEN;
            sess->tsizeReceived = false;
            sess->requestedFileSize = 0;
            sess->retryCount = 0;
            // What has been sent; -1 - error packet, 0 - OACK packet, 1+ - current file packet
            //            sess->currentBuffer = array;
            sess->timeoutTimer.setSingleShot(true);
            sess->timeoutTimer.setInterval(TFTP_TIMEOUT);

            connect(&sess->timeoutTimer, &QTimer::timeout, this, std::bind(&TFTPServer::timeout, this, sess));
            m_mutex.lock();
            m_clients.insert(client_key, sess);
            m_mutex.unlock();
        }
        // Check the request type
        if (array.size() < 4) {
            reportError(sess, "invalid request", "Invalid request received (less than 4 bytes), ignored");
            reset(sess);
            continue;
        }
        struct tftp_header* th = (struct tftp_header*)array.data();
        switch (qFromBigEndian(th->opcode)) {
            case TFTP_OP_RRQ:  //文件名字段
                sess->requestedFilename.clear();
                sess->tsizeReceived = false;
                sess->requestBlockSize = TFTP_DATALEN;
                sess->requestedFile.close();
                sess->currentBlock = -1;
                sess->retryCount = 0;
                sess->timeoutTimer.stop();
                sess->timeoutTimer.start(TFTP_TIMEOUT);
                parseReadRequest(sess, array);
                break;
            case TFTP_OP_WRQ:  // 文件写请求
                {
                sess->requestedFilename.clear();
                sess->tsizeReceived = false;
                sess->requestBlockSize = TFTP_DATALEN;
                sess->requestedFile.close();
                sess->currentBlock = -1;
                sess->retryCount = 0;
                sess->timeoutTimer.stop();
                //sess->timeoutTimer.start(TFTP_TIMEOUT);
                parseWriteRequest(sess, array);
                }
                break;
            case TFTP_OP_DATA:{
                    quint64 received;
                    received = array.size();
                    if (qFromBigEndian(th->data.block) != sess->currentBlock) {
                        qDebug()<<"qFromBigEndian(th->data.block) != block";
                        qDebug()<<"qFromBigEndian(th->data.block):"<<qFromBigEndian(th->data.block);
                        qDebug()<<"block:"<<sess->currentBlock;

                        sess->retryCount++;
                        break;
                    }
                    if (sess->retryCount > TFTP_RETRIES) return;

                    sess->requestedFile.write(array.data() + sizeof(struct tftp_header), received - sizeof(struct tftp_header));
                    sendAck(sess,sess->currentBlock++);

                    if(received != TFTP_DATALEN + sizeof(struct tftp_header))
                    {
                        emit fileTransferFinished(RTN_OK,sess->requestedFilename);
                        qDebug("received %d blocks, %llu bytes", sess->currentBlock - 1, (sess->currentBlock - 2) * TFTP_DATALEN + received-sizeof(struct tftp_header));
                        reset(sess);
                    }
                }break;
            case TFTP_OP_ACK: {
                // What has been acknowledged?
                if (sess->currentBlock == -1) {
                    qDebug() << ("TFTP server: error request acknowledged");
                    break;
                }
                // Which block acknowledged?
                unsigned char* data = (unsigned char*)array.constData();

                unsigned int block = (data[2] << 8) | data[3];
#if defined(DUMP_TRAFFIC)
                qDebug() << (QString("[%2]TFTP server: ACK request acknowledged block %1").arg(block).arg(sess->clientKey));
#endif
                if ((unsigned int)sess->currentBlock != block) {
                    qDebug() << (QString("[%3]TFTP server: Invalid block acknowledged (%1 vs %2), ignoring")
                                     .arg(block)
                                     .arg(sess->currentBlock)
                                     .arg(sess->clientKey));
                    break;
                }

                sess->retryCount = 0;

                if (sess->currentBlock == 0) {
#if defined(DUMP_TRAFFIC)
                    qDebug() << QString("[%1]TFTP server: OACK request acknowledged, sending the file").arg(sess->clientKey);
#endif
                    sess->currentBlock = 1;
                } else
                    sess->currentBlock++;

                sendCurrentBlock(sess);
            } break;
            default: {
                reportError(sess, "invalid request", "Non-read request received, ignored");
                reset(sess);
            } break;
        }
    }
}
bool TFTPServer::parseWriteRequest(TFTPSenssion* sess, const QByteArray& array) {
    int zoffset = 2;
    int count = 0;
    QString optname, requestedMode;
    while (zoffset < array.size()) {
        int znext = array.indexOf('\0', zoffset);

        if (znext == -1) {
            reportError(sess, "invalid request", "Incomplete or invalid request received, ignored");
            return false;
        }

        // Which object is it?
        QString value = QString::fromLatin1(array.data() + zoffset, znext - zoffset);
        //        qDebug() << (QString("TFTP Server: parsed request string: %1").arg(value));

        if (count == 0)
            sess->requestedFilename = value;  // File name is the first
        else if (count == 1)
            requestedMode = value;  // Mode is the 1nd
        else if ((count % 2) == 0) {
            // The even element is an option name
            optname = value;
        } else {
            // The odd element is the parameter; we only support blocksize and tsize
            if (optname == "blksize") sess->requestBlockSize = value.toUInt();
            if (optname == "tsize"){
                sess->requestedFileSize = value.toInt();
                sess->tsizeReceived = true;
            }
            optname.clear();
        }

        count++;
        zoffset = znext + 1;
    }

    // As long as we got all we want, continue
    if (count < 2) {
        reportError(sess, "invalid request", "Incomplete request received, ignored");
        return false;
    }
    qDebug() << QString("[%1]%2,mode:%3,blksize:%4,tsize:%5")
                    .arg(sess->clientKey, sess->requestedFilename, requestedMode)
                    .arg(sess->requestBlockSize)
                    .arg(sess->tsizeReceived);
    // Check the request mode
    if (requestedMode != "octet") {
        reportError(sess, "invalid request", QString("Unsupported non-octet request mode '%1' received, ignored").arg(requestedMode));
        return false;
    }

    // Security check
    if (sess->requestedFilename.indexOf("../") != -1) {
        reportError(sess, "invalid file request", QString("Invalid file request: file %1 contains ../, not allowed").arg(sess->requestedFilename));
        return false;
    }

    // Try to find the requested file if it is a mask
    if (m_allowMasks && sess->requestedFilename.indexOf('*') != -1) {
//        QStringList masks;
//        masks << sess->requestedFilename;

//        QStringList files = m_rxDir.entryList(masks, QDir::Files, QDir::Name);

//        if (files.isEmpty()) {
//            reportError(sess, "nonexistent file requested", QString("Invalid file request: file mask %1 doesn't match any files").arg(sess->requestedFilename));

//            return false;
//        }

//        emit infoMessage(QString("File request: file mask %1 matched file %2").arg(sess->requestedFilename, files[0]));
//        sess->requestedFile.setFileName(m_rxDir.absolutePath() + QDir::separator() + files[0]);
    } else {
        uint32_t ip = sess->clientAddr.toIPv4Address();
        sess->requestedFile.setFileName(m_rxDir.absolutePath() + QDir::separator() + sess->requestedFilename + QString("_%1").arg(ip&0xFF));
    }

    // Could the file be opened?
    if (!sess->requestedFile.open(QIODevice::WriteOnly)) {
        reportError(sess, "invalid file request",
                    QString("Invalid file request: cannot open file %1: %2").arg(sess->requestedFilename, sess->requestedFile.errorString()));
        return false;
    }
    //    if (m_requestedFilename != "CMU") return false;
    // TFTP does not support 0-byte file transfer
    // sess->requestedFileSize = sess->requestedFile.size();

//    if (sess->requestedFileSize == 0) {
//        reportError(sess, "invalid file request", QString("Invalid file request: file %1 is empty").arg(sess->requestedFilename));
//        return false;
//    }

    // Does the file fit into the maximum number of requested blocks?
//    if (sess->requestedFileSize / sess->requestBlockSize > 65533) {
//        reportError(sess, "file too large",
//                    QString("Requested file %1 is too large to be sent with the %2 byte block size").arg(sess->requestedFilename).arg(sess->requestBlockSize));
//        return false;
//    }

//    if (sess->requestBlockSize != TFTP_DATALEN || sess->tsizeReceived) {

//    } else
//        sess->currentBlock = 1;
//    sess->currentBlock = 0;
//    sendCurrentBlock(sess);
    sendAck(sess,0);
    sess->currentBlock=1;
    return true;
}
bool TFTPServer::parseReadRequest(TFTPSenssion* sess, const QByteArray& array) {
    int zoffset = 2;
    int count = 0;
    QString optname, requestedMode;
    while (zoffset < array.size()) {
        int znext = array.indexOf('\0', zoffset);

        if (znext == -1) {
            reportError(sess, "invalid request", "Incomplete or invalid request received, ignored");
            return false;
        }

        // Which object is it?
        QString value = QString::fromLatin1(array.data() + zoffset, znext - zoffset);
        //        qDebug() << (QString("TFTP Server: parsed request string: %1").arg(value));

        if (count == 0)
            sess->requestedFilename = value;  // File name is the first
        else if (count == 1)
            requestedMode = value;  // Mode is the 1nd
        else if ((count % 2) == 0) {
            // The even element is an option name
            optname = value;
        } else {
            // The odd element is the parameter; we only support blocksize and tsize
            if (optname == "blksize") sess->requestBlockSize = value.toUInt();
            if (optname == "tsize") sess->tsizeReceived = true;
            optname.clear();
        }

        count++;
        zoffset = znext + 1;
    }

    // As long as we got all we want, continue
    if (count < 2) {
        reportError(sess, "invalid request", "Incomplete request received, ignored");
        return false;
    }
    qDebug() << QString("[%1]%2,mode:%3,blksize:%4,tsize:%5")
                    .arg(sess->clientKey, sess->requestedFilename, requestedMode)
                    .arg(sess->requestBlockSize)
                    .arg(sess->requestedFileSize);
    // Check the request mode
    if (requestedMode != "octet") {
        reportError(sess, "invalid request", QString("Unsupported non-octet request mode '%1' received, ignored").arg(requestedMode));
        return false;
    }

    // Security check
    if (sess->requestedFilename.indexOf("../") != -1) {
        reportError(sess, "invalid file request", QString("Invalid file request: file %1 contains ../, not allowed").arg(sess->requestedFilename));
        return false;
    }

    // Try to find the requested file if it is a mask
    if (m_allowMasks && sess->requestedFilename.indexOf('*') != -1) {
        QStringList masks;
        masks << sess->requestedFilename;

        QStringList files = m_rootDir.entryList(masks, QDir::Files, QDir::Name);

        if (files.isEmpty()) {
            reportError(sess, "nonexistent file requested", QString("Invalid file request: file mask %1 doesn't match any files").arg(sess->requestedFilename));

            return false;
        }

        emit infoMessage(QString("File request: file mask %1 matched file %2").arg(sess->requestedFilename, files[0]));
        sess->requestedFile.setFileName(m_rootDir.absolutePath() + QDir::separator() + files[0]);
    } else {
        sess->requestedFile.setFileName(m_rootDir.absolutePath() + QDir::separator() + sess->requestedFilename);
    }

    // Could the file be opened?
    if (!sess->requestedFile.open(QIODevice::ReadOnly)) {
        reportError(sess, "invalid file request",
                    QString("Invalid file request: cannot open file %1: %2").arg(sess->requestedFilename, sess->requestedFile.errorString()));

        return false;
    }
    //    if (m_requestedFilename != "CMU") return false;
    // TFTP does not support 0-byte file transfer
    sess->requestedFileSize = sess->requestedFile.size();

    if (sess->requestedFileSize == 0) {
        reportError(sess, "invalid file request", QString("Invalid file request: file %1 is empty").arg(sess->requestedFilename));
        return false;
    }

    // Does the file fit into the maximum number of requested blocks?
    if (sess->requestedFileSize / sess->requestBlockSize > 65533) {
        reportError(sess, "file too large",
                    QString("Requested file %1 is too large to be sent with the %2 byte block size").arg(sess->requestedFilename).arg(sess->requestBlockSize));
        return false;
    }

    if (sess->requestBlockSize != TFTP_DATALEN || sess->tsizeReceived) {
        sess->currentBlock = 0;
    } else
        sess->currentBlock = 1;

    sendCurrentBlock(sess);
    return true;
}

void TFTPServer::timeout(TFTPSenssion* sess) {
    sess->retryCount++;

    if (sess->retryCount < TFTP_RETRIES) {
        qDebug() << (QString("TFTP server: timed out; resending the block %1, %2nd attempt").arg(sess->currentBlock).arg(sess->retryCount));
        sendCurrentBlock(sess);
        sess->timeoutTimer.start();
    } else {
        emit infoMessage("File operation timed out and retry count exceeded; aborting");
        setStatus(tr("Running: connection timed out"));
        emit fileTransferFinished(RTN_FAILED, QString("[%1]%2").arg(sess->clientKey, sess->requestedFilename));
        qDebug() << QString("[%1]fileTransferFinished FAILED,%2").arg(sess->clientKey, "File operation timed out and retry count exceeded; aborting");
        reset(sess);
    }
}

void TFTPServer::sendCurrentBlock(TFTPSenssion* sess) {
    if (sess->currentBlock == 0) {
        // Send the OACK
        unsigned int size = 2;

        if (sess->tsizeReceived) size += 7 + QString::number(sess->requestedFileSize).length();

        if (sess->requestBlockSize != TFTP_DATALEN) size += 9 + QString::number(sess->requestBlockSize).length();

        // Create the OACK message: RFC 2347
        // +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+
        // |  opc  |  opt1  | 0 | value1 | 0 |  optN  | 0 | valueN | 0 |
        // +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+
        //
        QByteArray array(size, 0);
        array.data()[1] = TFTP_OP_OACK;
        int offset = 2;

        if (sess->tsizeReceived) {
            strncpy(array.data() + offset, "tsize", 6);
            offset += 6;

            strcpy(array.data() + offset, QString::number(sess->requestedFileSize).toStdString().c_str());
            offset += QString::number(sess->requestedFileSize).length() + 1;
        }

        if (sess->requestBlockSize != TFTP_DATALEN) {
            strncpy(array.data() + offset, "blksize", 8);
            offset += 8;

            strcpy(array.data() + offset, QString::number(sess->requestBlockSize).toStdString().c_str());
        }

        //        Logger::log("TFTP server: sent OACK message");
        sendArray(sess, array);
    } else {
        // The first block means the file offset is 0
        qint64 offset = (sess->currentBlock - 1) * sess->requestBlockSize;

        if (offset > sess->requestedFileSize) {
            emit infoMessage(tr("File %1 has been transferred successfully").arg(sess->requestedFilename));
            setStatus(tr("Running: transfer completed"));
            emit fileTransferFinished(RTN_OK, QString("[%1]%2").arg(sess->clientKey, sess->requestedFilename));
            qDebug() << QString("[%1]fileTransferFinished OK,%2").arg(sess->clientKey, "File has been transferred successfully");
            reset(sess);
            return;
        }

        // Create the data message: RFC 1350
        //
        //  2 bytes     2 bytes      n bytes
        //  ----------------------------------
        // | Opcode |   Block #  |   Data     |
        //  ----------------------------------
        //
        unsigned int reminder = qMin((unsigned int)(sess->requestedFileSize - offset), sess->requestBlockSize);

        QByteArray array(4 + reminder, 0);
        array.data()[1] = TFTP_OP_DATA;
        array.data()[2] = sess->currentBlock >> 8;
        array.data()[3] = sess->currentBlock & 0xFF;

        if (sess->requestedFile.read(array.data() + 4, reminder) != reminder) {
            reportError(sess, "invalid file seek/read", QString("Error reading the file"));
            emit fileTransferFinished(RTN_FAILED, QString("[%1]%2").arg(sess->clientKey, sess->requestedFilename));
            qDebug() << QString("[%1]fileTransferFinished FAILED,%2").arg(sess->clientKey, "invalid file seek/read");
            reset(sess);
            return;
        }
#if defined(DUMP_TRAFFIC)
        qDebug() << (QString("TFTP server: sent out %1 out of %2, block %3").arg(offset).arg(sess->requestedFileSize).arg(sess->currentBlock));
#endif
        setStatus(tr("Running: sending the file"));
        emit fileTransferProgress((unsigned int)offset, (unsigned int)sess->requestedFileSize);
        sendArray(sess, array);
    }

    sess->timeoutTimer.start();
}

void TFTPServer::sendArray(TFTPSenssion* sess, const QByteArray& array) {
#if defined(DUMP_TRAFFIC)
    qDebug() << ("TFTP server: data sent:", array.toHex());
#endif
    m_mutex.lock();
    m_socket.writeDatagram(array, sess->clientAddr, sess->clientPort);
    m_mutex.unlock();
}

QString TFTPServer::status() const { return m_statusText; }

void TFTPServer::sendAck(TFTPSenssion* sess,quint16 block) {
    struct tftp_header ack;
    ack.opcode = qToBigEndian((quint16)B_ACK);
    ack.data.block = qToBigEndian(block);
    m_socket.writeDatagram((char *)&ack, sizeof(struct tftp_header), sess->clientAddr, sess->clientPort);
}

//void TFTPServer::nak(TFTPSenssion* sess,TftpError error) {
//    struct tftp_header *th = (struct tftp_header *)sess->buffer;
//    th->opcode = qToBigEndian((quint16)B_ERROR);
//    th->data.block = qToBigEndian((quint16)error);

//    struct errmsg *pe;
//    for (pe = errmsgs; pe->e_code >= 0; pe++)
//        if (pe->e_code == error) break;
//    if (pe->e_code < 0) {
//        pe->e_msg = strerror(error - 100);
//        th->data.block = EUNDEF; /* set 'undef' errorcode */
//    }

//    strcpy(th->data.data, pe->e_msg);
//    int length = strlen(pe->e_msg);
//    th->data.data[length] = 0;
//    length += 5;
//    m_socket.writeDatagram(sess->buffer, length, sess->clientAddr, sess->clientPort);
//}
