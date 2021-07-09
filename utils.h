#ifndef UTILS_H
#define UTILS_H
#include <time.h>

#define TIME_OUTOFDATE 24 * 31 * 24 * 60 * 60
#include <QByteArray>
time_t cvt_TIME(char const *Date);
QByteArray gzipCompress(const QByteArray &in);
QByteArray gzipUncompress(const QByteArray &data);
int FindFile(const QString &_filePath);
bool isDirExist(QString fullPath);
#endif  // UTILS_H
