#include <QFile>
#include <QFileInfo>
#include <QtEndian>

#include "Qtftp.h"

Qtftp::Qtftp() {
    connect(this, SIGNAL(doGet(QString, QString)), this, SLOT(client_get(QString, QString)));
    connect(this, SIGNAL(doPut(QString, QString)), this, SLOT(client_put(QString, QString)));
}

void Qtftp::server() {
    qDebug("Starting server");
    sock = new QUdpSocket(this);
    connect(&worker, SIGNAL(finished()), sock, SLOT(deleteLater()));
    connect(this, SIGNAL(error(int)), sock, SLOT(deleteLater()));
    if (!sock->bind(PORT)) {
        sock->close();
        emit error(BindError);
        return;
    }
    while (true) {
        sock->waitForReadyRead(-1);
        qint64 readed = sock->readDatagram(buffer, SEGSIZE + sizeof(struct tftp_header), &rhost, &rport);
        if (readed < 0) continue;

        struct tftp_header *th = (struct tftp_header *)buffer;
        switch (qFromBigEndian(th->opcode)) {
            case B_RRQ:
                server_get();
                break;
            case B_WRQ:
                server_put();
                break;
            default:
                nak(EBADOP);
        }
    }
    sock->close();
    delete sock;
}

void Qtftp::startServer() {
    moveToThread(&worker);
    connect(this, SIGNAL(doServer()), this, SLOT(server()));
    worker.start();
    emit doServer();
}

void Qtftp::stopServer() {
    worker.terminate();
    sock->close();
    sock->deleteLater();
}

void Qtftp::server_get() {
    struct tftp_header *th = (struct tftp_header *)buffer;
    QString path = th->path;
    qDebug("sending %s", th->path);
    if (QString(th->path).contains('/')) {
        nak(EACCESS);
        return;
    }
    QFile file(th->path);
    if (!file.open(QIODevice::ReadOnly)) switch (file.error()) {
            case QFile::OpenError:
                nak(ENOTFOUND);
                return;
            case QFile::PermissionsError:
                nak(EACCESS);
                return;
            default:
                nak(EUNDEF);
                return;
        }

    emit send(true);
    quint64 readed;
    quint16 block = 1;
    do {
        qint64 blocks = file.size() / 512;
        int percent = -1;

        th->opcode = qToBigEndian((quint16)B_DATA);
        th->data.block = qToBigEndian((quint16)block);
        readed = file.read(buffer + sizeof(struct tftp_header), SEGSIZE);

        int i;
        for (i = 0; i < RETRIES; i++) {
            sock->writeDatagram(buffer, readed + sizeof(struct tftp_header), rhost, rport);
            if (waitForAck(block++)) break;
        }
        if (i == RETRIES) return;
        int newp = block * 100 / blocks;
        if (newp > percent) {
            percent = newp;
            emit progress(newp);
        }
    } while (readed == SEGSIZE);
    emit fileSent(Ok, path);
    qDebug("sent %d blocks, %llu bytes", (block - 1), (block - 2) * SEGSIZE + readed);
}

void Qtftp::server_put() {
    struct tftp_header *th = (struct tftp_header *)buffer;
    QString path = th->path;
    qDebug("receiving %s", th->path);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) switch (file.error()) {
            case QFile::PermissionsError:
                nak(EACCESS);
                return;
            default:
                nak(EUNDEF);
                return;
        }

    sendAck(0);
    quint64 received;
    quint16 block = 1;
    int i;
    do {
        for (i = 0; i < RETRIES; i++) {
            QHostAddress h;
            quint16 p;
            if (!sock->waitForReadyRead(TIMEOUT)) return;
            received = sock->readDatagram(buffer, SEGSIZE + sizeof(struct tftp_header), &h, &p);

            if (h != rhost || p != rport) continue;

            if (th->opcode == qToBigEndian((quint16)B_DATA) && qFromBigEndian(th->data.block) == block) break;
        }
        if (i == RETRIES) return;
        file.write(buffer + sizeof(struct tftp_header), received - sizeof(struct tftp_header));
        sendAck(block++);
    } while (received == SEGSIZE + sizeof(struct tftp_header));
    emit fileReceived(path);
    qDebug("received %d blocks, %llu bytes", block - 1, (block - 2) * SEGSIZE + received);
}

void Qtftp::client_get(QString path, QString server) {
    qDebug("receiving %s", path.toUtf8().constData());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;

    QFileInfo name(path);
    struct tftp_header *th = (struct tftp_header *)buffer;
    strcpy(th->path, name.fileName().toUtf8().constData());
    strcpy(th->path + name.fileName().length() + 1, "octet");

    sock = new QUdpSocket(this);
    sock->bind();
    connect(&worker, SIGNAL(finished()), sock, SLOT(deleteLater()));
    connect(this, SIGNAL(error(int)), sock, SLOT(deleteLater()));

    th->opcode = qToBigEndian((quint16)B_RRQ);

    if (sock->writeDatagram(buffer, sizeof(struct tftp_header) + name.fileName().length() + sizeof("octet") - 1, QHostAddress(server), PORT) <= 0) {
        emit error(NetworkError);
        return;
    }

    qint64 readed;
    quint16 block = 1;
    do {
        if (!sock->waitForReadyRead(TIMEOUT)) {
            emit error(Timeout);
            return;
        }
        readed = sock->readDatagram(buffer, SEGSIZE + (sizeof(struct tftp_header)), &rhost, &rport);

        file.write(th->data.data, readed - sizeof(struct tftp_header));
        sendAck(block++);
    } while (readed == SEGSIZE + sizeof(tftp_header));
    qDebug("received %d blocks, %llu bytes", block - 1, (block - 2) * SEGSIZE + readed);
    sock->close();
    delete sock;
    emit fileReceived(name.fileName());
}

void Qtftp::client_put(QString path, QString server) {
    qDebug("sending %s", path.toUtf8().constData());
    QFile file(path);
    QFileInfo name(path);
    if (!file.open(QIODevice::ReadOnly)) {
        fileSent(FileError, name.fileName());
        return;
    }

    memset(buffer, 0, sizeof(buffer));
    struct tftp_header *th = (struct tftp_header *)buffer;
    int buf_len = 0;
    strncpy(th->path, name.fileName().toUtf8().constData(), name.fileName().length());
    buf_len += name.fileName().length() + 1;
    strncpy(th->path + buf_len, "octet", sizeof("octet"));
    buf_len += sizeof("octet");  //使用sizeof 包含'\0'
    qDebug() << name.fileName();
    sock = new QUdpSocket(this);
    sock->bind();
    connect(&worker, SIGNAL(finished()), sock, SLOT(deleteLater()));
    connect(this, SIGNAL(error(int)), sock, SLOT(deleteLater()));

    th->opcode = qToBigEndian((quint16)B_WRQ);

    int i;
    for (i = 0; i < RETRIES; i++) {
        if (sock->writeDatagram(buffer, sizeof(th->opcode) + buf_len, QHostAddress(server), PORT) <= 0) {
            emit error(NetworkError);
            fileSent(NetworkError, name.fileName());
            return;
        }
        rhost.clear();
        rport = 0;
        if (waitForAck(0)) break;
    }
    if (i == RETRIES) {
        emit error(Timeout);
        fileSent(Timeout, name.fileName());
        return;
    }

    quint64 readed;
    quint16 block = 1;
    do {
        qint64 blocks = file.size() / 512;
        int percent = -1;

        th->opcode = qToBigEndian((quint16)B_DATA);
        th->data.block = qToBigEndian((quint16)block);
        readed = file.read(buffer + sizeof(struct tftp_header), SEGSIZE);

        for (i = 0; i < RETRIES; i++) {
            if (sock->writeDatagram(buffer, readed + sizeof(struct tftp_header), rhost, rport) <= 0) {
                emit error(NetworkError);
                fileSent(NetworkError, name.fileName());
                return;
            }
            if (waitForAck(block++)) break;
        }
        if (i == RETRIES) {
            emit error(Timeout);
            fileSent(Timeout, name.fileName());
            return;
        }
        int newp = block * 100 / blocks;
        if (newp > percent) {
            percent = newp;
            emit progress(newp);
        }
    } while (readed == SEGSIZE);
    qDebug("sent %d blocks, %llu bytes", (block - 1), (block - 2) * SEGSIZE + readed);
    sock->close();
    delete sock;
    emit fileSent(Ok, name.fileName());
}

void Qtftp::get(QString path, QString server) {
    moveToThread(&worker);
    worker.start();
    emit doGet(path, server);
}

void Qtftp::put(QString path, QString server) {
    moveToThread(&worker);
    worker.start();
    qDebug() << "send " << path << " to " << server;
    emit doPut(path, server);
}

bool Qtftp::isRunning() { return worker.isRunning(); }

bool Qtftp::waitForAck(quint16 block) {
    for (int i = 0; i < RETRIES; i++) {
        struct tftp_header th;
        QHostAddress h;
        quint16 p;

        if (!sock->waitForReadyRead(TIMEOUT)) continue;

        sock->readDatagram((char *)&th, sizeof(struct tftp_header), &h, &p);
        if (rhost.isNull() && rport == 0) {
            rhost = h;
            rport = p;
        } else if (h != rhost || p != rport)
            continue;

        if (th.opcode == qToBigEndian((quint16)B_ACK) && qFromBigEndian(th.data.block) == block) return true;
    }
    return false;
}

void Qtftp::sendAck(quint16 block) {
    struct tftp_header ack;
    ack.opcode = qToBigEndian((quint16)B_ACK);
    ack.data.block = qToBigEndian(block);
    sock->writeDatagram((char *)&ack, sizeof(struct tftp_header), rhost, rport);
}

void Qtftp::nak(TftpError error) {
    struct tftp_header *th = (struct tftp_header *)buffer;
    th->opcode = qToBigEndian((quint16)B_ERROR);
    th->data.block = qToBigEndian((quint16)error);

    struct errmsg *pe;
    for (pe = errmsgs; pe->e_code >= 0; pe++)
        if (pe->e_code == error) break;
    if (pe->e_code < 0) {
        pe->e_msg = strerror(error - 100);
        th->data.block = EUNDEF; /* set 'undef' errorcode */
    }

    strcpy(th->data.data, pe->e_msg);
    int length = strlen(pe->e_msg);
    th->data.data[length] = 0;
    length += 5;
    sock->writeDatagram(buffer, length, rhost, rport);
}
