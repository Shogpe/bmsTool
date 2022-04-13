#ifndef UTILS_H
#define UTILS_H
#include <time.h>
#define MODBUS_GET_INT32_FROM_INT16_SWAP(tab_int16, index) ((tab_int16[(index) + 1] << 16) + tab_int16[(index)])
#define TIME_OUTOFDATE 24 * 31 * 24 * 60 * 60
#include <QByteArray>
time_t cvt_TIME(char const *Date);
QByteArray gzipCompress(const QByteArray &in);
QByteArray gzipUncompress(const QByteArray &data);
int FindFile(const QString &_filePath);
int log2csv();
bool isDirExist(QString fullPath);
#endif  // UTILS_H
