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
typedef struct {
    uint16_t Idc[20];
    uint16_t Udc[20];
    uint16_t Ile[20];
    uint16_t Rins[20];
    uint16_t SysSta[5];
    uint16_t ErrStatus[5];
    uint16_t WarnStatus[5];
    //    uint16_t CalData[18];
    uint16_t u16MaxCellVolt;    //单体电池电压最大值x10000
    uint16_t u16MaxCellVoltId;  //单体电池电压最大值ID

    uint16_t u16MinCellVolt;    //单体电池电压最小值
    uint16_t u16MinCellVoltId;  //单体电池电压最小值ID

    int16_t i16MaxPackTemp;     //电池模组温度最大值 	x10
    uint16_t u16MaxPackTempId;  //电池模组温度最大值ID

    int16_t i16MinPackTemp;     //电池模组温度最小值
    uint16_t u16MinPackTempId;  //电池模组温度最小值ID

    int16_t i16MaxPoleTemp;     // PACK极柱温度最大值
    uint16_t u16MaxPoleTempId;  // PACK极柱温度最大值ID

    uint16_t u16MaxCellVoltDiff;  //最大单体电压差值
    int16_t i16MaxPackTempDiff;   //最大电池模组温差值

    uint16_t u16MaxTRiseRate;    //电池模组最大温度上升速率
    uint16_t u16MaxTRiseRateId;  //电池模组最大温度上升速率ID

    uint16_t u16MaxPackVolt;    //最大模组电压
    uint16_t u16MaxPackVoltId;  //最大模组电压ID
    uint16_t u16AvgCellVolt;    //平均单体电压

    uint16_t u16CPoTWireSta;  //簇极柱温度断线状态
} _log_st;
#define DATA_LEN sizeof(_log_st)
typedef struct {
    uint32_t time;
    uint16_t len;
    uint16_t time_ms;
    union {
        uint16_t arr[DATA_LEN / 2];
        _log_st st;
    } data;
} CMU_LOG;
#define GET_BIT(x, bit) (((x) & (1 << (bit))) >> (bit))
QString getStatusString(uint16_t status) {
    QStringList statusList;
    if (GET_BIT(status, 0)) statusList << "总故障";
    if (GET_BIT(status, 1)) statusList << "总告警";
    if (GET_BIT(status, 2)) statusList << "充满";
    if (GET_BIT(status, 3)) statusList << "放空";
    if (GET_BIT(status, 4)) statusList << "未初始化";
    if (GET_BIT(status, 5)) statusList << "通信故障";
    if (GET_BIT(status, 6)) statusList << "均衡";
    if (GET_BIT(status, 7)) statusList << "充电";
    if (GET_BIT(status, 8)) statusList << "放电";
    if (GET_BIT(status, 9)) statusList << "停机";
    if (GET_BIT(status, 10)) statusList << "升级";
    if (GET_BIT(status, 11)) statusList << "绝缘通信故障";
    if (GET_BIT(status, 12)) statusList << "自检故障";
    if (GET_BIT(status, 13)) statusList << "拨码故障";
    if (GET_BIT(status, 14)) statusList << "BMU故障";
    if (GET_BIT(status, 15)) statusList << "并网";
    //    if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
    return statusList.join("|");
}
int log2csv() {
    QByteArray data;
    // 烧写
    QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Read "), "", "");
    if (fileName.isEmpty()) {
        return -1;
    }
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Error: Cannot read file: " << qPrintable(file.errorString());
        return -1;
    }
    data = file.readAll();
    file.close();
    int len = data.size() / sizeof(CMU_LOG);
    CMU_LOG *log = (CMU_LOG *)data.data();
    if ((data.size() % sizeof(CMU_LOG) == 0) || (data.size() / log->len == 0)) {
    }
    std::vector<double_t> Idc;
    std::vector<double_t> Udc;
    std::vector<double_t> Ile;
    std::vector<double_t> Rins;

    std::vector<uint16_t> SysSta;
    std::vector<uint16_t> ErrStatus;
    std::vector<uint16_t> WarnStatus;

    uint16_t CalData[18];
    while (len--) {
        //        qDebug() << log->time << "." << log->time_ms << log->len<<getStatusString(log->data.st.SysSta);
        for (int i = 0; i < 20; i++) {
            Idc.push_back(log->data.st.Idc[i] * 0.1);
            if (i % 5) {
                qDebug() << ((uint64_t)log->time) * 1000 + log->time_ms + i * 50
                         << getStatusString(log->data.st.SysSta[i % 5]);
            }
        }
        log++;
    }
    return 0;
}
