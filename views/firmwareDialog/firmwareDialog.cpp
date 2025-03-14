#include "firmwareDialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include "crc32.h"
#include "myhelper.h"
#include "ui_firmwareDialog.h"
#include "db_manager.h"

firmwareDialog::firmwareDialog(QWidget* parent) : QDialog(parent), ui(new Ui::firmwareDialog) {
    state = 0;
    ui->setupUi(this);
    // remove question mark from the title bar
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("更新档资讯查看"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}
firmwareDialog::~firmwareDialog() { delete ui; }
//

void firmwareDialog::on_transmitBrowse_clicked() {
    ui->transmitPath->setText(QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("任意文件 (*.*)")));
    if (ui->transmitPath->text().isEmpty()) return;
    if (ui->transmitPath->text().isEmpty() != true && (loadfile(ui->transmitPath->text()) > 0)) {
        //        ui->transmitButton->setEnabled(true);
    } else {
        //        ui->transmitButton->setDisabled(true);
        myHelper::ShowMessageBoxError(tr("文件加载失败！"));
    }
}
bool checkSignV1(QByteArray data, FIRMWARE_HEAER& hdr_tmp, FIRMWARE_TAIL& tail) {
    FIRMWARE_HEAER* hdr = NULL;
    int is_head_valid = 0;
    //成功读取文件,扫描头部信息
    uint32_t* p32 = (uint32_t*)data.data();
    for (int i = 0; i < data.size() / 4; i++) {
        if (*p32 == APP_FIRMWARE_HEAD) {
            hdr = (FIRMWARE_HEAER*)p32;
            if (hdr->head_len == sizeof(FIRMWARE_HEAER)) {
                qDebug() << QString("").sprintf("Version:0x%x,Compile time:%s %s,FirmwareType:%d",
                                                hdr->firmware_version, hdr->RELEASEDATE, hdr->RELEASETIME,
                                                hdr->head_type)
                         << QString("").sprintf(
                                "head_len:%d,check_sum:0x%04x,"
                                "firmware_crc32:0x%04x,firmware_size:%d bytes",
                                hdr->head_len, hdr->check_sum, hdr->firmware_crc32, hdr->firmware_size);
                is_head_valid = i;
                break;
            }
        }
        p32++;
    }
    if (is_head_valid) {
        //头部合法，计算CRC
        if (data.size() >= hdr->firmware_size) {
            memcpy(&hdr_tmp, hdr, sizeof(FIRMWARE_HEAER));
            hdr->firmware_size = 0;
            hdr->firmware_crc32 = 0;
            hdr->check_sum = 0xFFFF;
            uint32_t crc = crc32_buf(0, data.data(), is_head_valid);
            int offset = is_head_valid + sizeof(FIRMWARE_HEAER);
            crc = crc32_buf(crc, data.data() + offset, (hdr_tmp.firmware_size - offset));
            qDebug() << "address:" << is_head_valid << sizeof(FIRMWARE_HEAER) << offset;

            if (hdr_tmp.firmware_crc32 != crc) {
                qDebug() << QObject::tr("文件校验错误");
                qDebug()
                    << QString("crc=%1,%2").arg(crc, 8, 16, QChar('0')).arg(hdr_tmp.firmware_crc32, 8, 16, QChar('0'))
                    << hdr_tmp.firmware_size;
                return false;
            }

            //头部信息校验
            hdr->firmware_size = hdr_tmp.firmware_size;
            hdr->firmware_crc32 = hdr_tmp.firmware_crc32;
            hdr->check_sum = 0;
            uint16_t* p16 = (uint16_t*)p32;
            for (int j = 0; j < (sizeof(FIRMWARE_HEAER) - sizeof(uint16_t)); j++) {
                hdr->check_sum += *(p16 + j);
            }
            if (hdr->check_sum != hdr_tmp.check_sum) {
                qDebug() << QObject::tr("固件信息校验错误");
                qDebug() << QString("check_sum=%1,%2")
                                .arg(hdr->check_sum, 4, 16, QChar('0'))
                                .arg(hdr_tmp.check_sum, 4, 16, QChar('0'));
                return false;
            }
            if (data.size() == hdr->firmware_size + sizeof(FIRMWARE_TAIL)) {
                memcpy(&tail, data.data() + hdr->firmware_size, sizeof(FIRMWARE_TAIL));
            }
            return true;
        }
    } else {
        qDebug() << ("can't find valid header.");
    }
    return false;
}
bool checkSignV2(QByteArray data, FIRMWARE_HEAER& hdr_tmp, FIRMWARE_TAIL& tail) {
    FIRMWARE_HEAER* hdr = NULL;
    int is_head_valid = 0;
    //成功读取文件,扫描头部信息
    uint32_t* p32 = (uint32_t*)data.data();
    for (int i = 0; i < data.size() / 4; i++) {
        if (*p32 == APP_FIRMWARE_HEAD) {
            hdr = (FIRMWARE_HEAER*)p32;
            if (hdr->head_len == sizeof(FIRMWARE_HEAER)) {
                qDebug() << QString("").sprintf("Version:0x%x,Compile time:%s %s,FirmwareType:%d",
                                                hdr->firmware_version, hdr->RELEASEDATE, hdr->RELEASETIME,
                                                hdr->head_type)
                         << QString("").sprintf(
                                "head_len:%d,check_sum:0x%04x,"
                                "firmware_crc32:0x%04x,firmware_size:%d bytes",
                                hdr->head_len, hdr->check_sum, hdr->firmware_crc32, hdr->firmware_size);
                is_head_valid = i;
                break;
            }
        }
        p32++;
    }
    if (is_head_valid) {
        //头部合法，计算CRC
        if (data.size() >= hdr->firmware_size) {
            memcpy(&hdr_tmp, hdr, sizeof(FIRMWARE_HEAER));
            //            hdr->firmware_size = 0;
            //            hdr->firmware_crc32 = 0;
            //            hdr->check_sum = 0xFFFF;
            uint32_t crc = crc32_buf(0, data.data(), is_head_valid * sizeof(uint32_t));
            int offset = is_head_valid * sizeof(uint32_t) + sizeof(FIRMWARE_HEAER);
            crc = crc32_buf(crc, data.data() + offset, (hdr_tmp.firmware_size - offset));
            qDebug() << "address:" << is_head_valid << sizeof(FIRMWARE_HEAER) << offset;

            if (hdr_tmp.firmware_crc32 != crc) {
                qDebug() << QObject::tr("文件校验错误");
                qDebug()
                    << QString("crc=%1,%2").arg(crc, 8, 16, QChar('0')).arg(hdr_tmp.firmware_crc32, 8, 16, QChar('0'))
                    << hdr_tmp.firmware_size;
                return false;
            }

            //头部信息校验

            uint16_t check_sum = 0;
            uint16_t* p16 = (uint16_t*)p32;
            for (int j = 0; j < (sizeof(FIRMWARE_HEAER) - sizeof(uint16_t)) / sizeof(uint16_t); j++) {
                check_sum += *(p16 + j);
            }
            if (check_sum != hdr->check_sum) {
                qDebug() << QObject::tr("固件信息校验错误");
                qDebug() << QString("check_sum=%1,%2")
                                .arg(check_sum, 4, 16, QChar('0'))
                                .arg(hdr->check_sum, 4, 16, QChar('0'));
                return false;
            }
            if (data.size() == hdr->firmware_size + sizeof(FIRMWARE_TAIL)) {
                memcpy(&tail, data.data() + hdr->firmware_size, sizeof(FIRMWARE_TAIL));
            }
            return true;
        }
    } else {
        qDebug() << ("can't find valid header.");
    }
    return false;
}
int firmwareDialog::loadfile(QString fileName) {
    //检查文件合法
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << qPrintable(file.errorString());
        return -1;
    }
    //读取文件至缓存
    m_filedata = file.readAll();
    int origin_filelen = m_filedata.size();
    if (origin_filelen < 1024 || origin_filelen > (1 * 1024 * 1024)) {
        qDebug() << "file length error!";
        return -2;
    }
    FIRMWARE_HEAER hdr;
    FIRMWARE_TAIL tail;
    if (checkSignV2(m_filedata, hdr, tail)) {
        qDebug() << QString("").sprintf(
            "sign by v2,head_len:%d,check_sum:0x%04x,FirmwareType:%d,"
            "firmware_crc32:0x%04x,firmware_size:%d bytes,Version:0x%x,Compile time:%s %s",
            hdr.head_len, hdr.head_type, hdr.check_sum, hdr.firmware_crc32, hdr.firmware_size, hdr.firmware_version,
            hdr.RELEASEDATE, hdr.RELEASETIME);
        ui->info->setText(tr("签名v2, 更新档类型 %1, 更新档版本 %2, 更新档长度 %3 Bytes, 发布日期 %4 %5")
                              .arg(hdr.head_type)
                              .arg(myHelper::IntegerToHexString(hdr.firmware_version))
                              .arg(hdr.firmware_size)
                              .arg(QString(hdr.RELEASEDATE), QString(hdr.RELEASETIME)));
    } else if (checkSignV1(m_filedata, hdr, tail)) {
        qDebug() << QString("").sprintf(
            "sign by v1,head_len:%d,check_sum:0x%04x,FirmwareType:%d,"
            "firmware_crc32:0x%04x,firmware_size:%d bytes,Version:0x%x,Compile time:%s %s",
            hdr.head_len, hdr.head_type, hdr.check_sum, hdr.firmware_crc32, hdr.firmware_size, hdr.firmware_version,
            hdr.RELEASEDATE, hdr.RELEASETIME);
        ui->info->setText(tr("签名v1, 更新档类型 %1, 更新档版本 %2, 更新档长度 %3 Bytes, 发布日期 %4 %5")
                              .arg(hdr.head_type)
                              .arg(myHelper::IntegerToHexString(hdr.firmware_version))
                              .arg(hdr.firmware_size)
                              .arg(QString(hdr.RELEASEDATE), QString(hdr.RELEASETIME)));
    } else {
        ui->info->setText(tr("文件校验错误,请确保固件正确！"));
    }
    //    ui->info->adjustSize();
    return 1;
}
QByteArray u32ToByte(uint32_t i) {
    QByteArray result;
    result.resize(4);
    result[3] = (uchar)(0x000000ff & i);
    result[2] = (uchar)((0x0000ff00 & i) >> 8);
    result[1] = (uchar)((0x00ff0000 & i) >> 16);
    result[0] = (uchar)((0xff000000 & i) >> 24);
    return result;
}

QByteArray u32ToByteR(uint32_t i) {
    QByteArray result;
    result.resize(4);
    result[0] = (uchar)(0x000000ff & i);
    result[1] = (uchar)((0x0000ff00 & i) >> 8);
    result[2] = (uchar)((0x00ff0000 & i) >> 16);
    result[3] = (uchar)((0xff000000 & i) >> 24);
    return result;
}
