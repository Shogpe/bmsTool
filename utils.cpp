#include "utils.h"
#include <QtZlib/zlib.h>
#include <stdio.h>
#include <string.h>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
QByteArray gzipUncompress(const QByteArray &src) {
    QByteArray outBuffer;
    z_stream strm;
    strm.zalloc = NULL;
    strm.zfree = NULL;
    strm.opaque = NULL;

    strm.avail_in = src.size();
    strm.next_in = (Bytef *)src.data();

    int err = -1, ret = -1;
    err = inflateInit2(&strm, MAX_WBITS + 16);
    if (err == Z_OK) {
        while (true) {
            char buffer[4096] = {0};
            strm.avail_out = 4096;
            strm.next_out = (Bytef *)buffer;
            int code = inflate(&strm, Z_FINISH);
            outBuffer.append(buffer, 4096 - strm.avail_out);
            // qDebug()<<"ddd"<<code<<Z_OK;
            if (Z_STREAM_END == code) {
                break;
            }
        }
    }
    inflateEnd(&strm);
    return outBuffer;
}

QByteArray gzipCompress(const QByteArray &in) {
    QByteArray outBuf;
    z_stream c_stream;
    int err = 0;
    int windowBits = 15;
    int GZIP_ENCODING = 16;
    if (!in.isEmpty()) {
        c_stream.zalloc = (alloc_func)0;
        c_stream.zfree = (free_func)0;
        c_stream.opaque = (voidpf)0;
        c_stream.next_in = (Bytef *)in.data();
        c_stream.avail_in = in.size();
        if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + GZIP_ENCODING, 8,
                         Z_DEFAULT_STRATEGY) != Z_OK)
            return QByteArray();
        for (;;) {
            char destBuf[4096] = {0};
            c_stream.next_out = (Bytef *)destBuf;
            c_stream.avail_out = 4096;
            int err = deflate(&c_stream, Z_FINISH);
            outBuf.append(destBuf, 4096 - c_stream.avail_out);
            if (err == Z_STREAM_END || err != Z_OK) {
                break;
            }
        }
        auto total = c_stream.total_out;
        deflateEnd(&c_stream);
        total = c_stream.total_out;
    }
    return outBuf;
}
QString d2String(double value, int precision) {
    QString str = QString::number(value, 'f', precision);
    while (str.back() == '0') {
        str.chop(1);
    }
    if (str.back() == '.') {
        str.chop(1);
    }
    return str;
}
int FindFile(const QString &_filePath) {
    QDir dir(_filePath);  // QDir的路径一定要是全路径，相对路径会有错误

    if (!dir.exists()) return -1;

    QStringList filters;
    filters << QString("*.rec");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);  //设置类型过滤器，只为文件格式
    dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式（后缀为.jpeg等图片格式）
    dir.setSorting(QDir::Time);
    //将其转化为一个list
    QFileInfoList list = dir.entryInfoList();
    if (list.size() < 1) return -1;
    QFile csv("output.csv");
    csv.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream out(&csv);
    int i = 0;
    QStringList keys;
    //在当前文件夹遍历文件
    do {
        QFileInfo fileInfo = list.at(i);
        QFile file(fileInfo.filePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("File open error ");
        }
        QByteArray b = file.read(file.size());
        QByteArray b1 = gzipUncompress(b);
        if (b1.size() < 100) continue;
        QJsonDocument jsonDoc = QJsonDocument::fromBinaryData(b1);

        if (i == 0) {
            keys = jsonDoc.object().keys();
            for (auto d : keys) {
                out << d << ",";
            }
            endl(out);
        }
        for (auto d : keys) {
            out << d2String(jsonDoc.object().value(d).toDouble(), 4) << ",";
        }
        endl(out);
        ++i;
    } while (i < list.size());
    csv.close();
    return 0;
}
bool isDirExist(QString fullPath) {
    QDir dir(fullPath);
    if (dir.exists()) {
        return true;
    } else {
        bool ok = dir.mkdir(".");  //创建多级目录
        return ok;
    }
}

