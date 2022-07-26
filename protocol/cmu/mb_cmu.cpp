#include "mb_cmu.h"
#include <QDebug>
#include <QJsonObject>
#include <QTimerEvent>
#include "myhelper.h"
#include "node_conf.h"
#include "utils.h"
static uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, 0x9000U, 0x1122, 0x3344, 0x5566};
const QString recPath = "Rec";
const QString dataPath = "Data";

mb_cmu::mb_cmu() {
    cmu = nullptr;
    csvfile = nullptr;
    stopDump = true;
    drv_status = 0;
    stop = false;
    mb_ip = "192.168.1.120";
    mb_port = 502;
    isDirExist(recPath);
    isDirExist(dataPath);
    pMq = MessageQueue::getInstance();
    pMq->registMsgQueue(0);
}
mb_cmu::mb_cmu(BMS_PROTOCOL ver) {
    cmu = nullptr;
    csvfile = nullptr;
    stopDump = true;
    drv_status = 0;
    stop = false;
    mb_ip = "192.168.1.120";
    mb_port = 502;
    protocal_ver = ver;
    isDirExist(recPath);
    isDirExist(dataPath);
    pMq = MessageQueue::getInstance();
    pMq->registMsgQueue(0);
}
QString mb_cmu::GetBalanceStatus(uint16_t status) {
    QStringList statusList;
    if ((((status >> 0) & 0x01) > 0)) statusList << "1";
    if ((((status >> 1) & 0x01) > 0)) statusList << "2";
    if ((((status >> 2) & 0x01) > 0)) statusList << "3";
    if ((((status >> 3) & 0x01) > 0)) statusList << "4";
    if ((((status >> 4) & 0x01) > 0)) statusList << "5";
    if ((((status >> 5) & 0x01) > 0)) statusList << "6";
    if ((((status >> 6) & 0x01) > 0)) statusList << "7";
    if ((((status >> 7) & 0x01) > 0)) statusList << "8";
    if ((((status >> 8) & 0x01) > 0)) statusList << "9";
    if ((((status >> 9) & 0x01) > 0)) statusList << "10";
    if ((((status >> 10) & 0x01) > 0)) statusList << "11";
    if ((((status >> 11) & 0x01) > 0)) statusList << "12";
    if ((((status >> 12) & 0x01) > 0)) statusList << "13";
    if ((((status >> 13) & 0x01) > 0)) statusList << "14";
    if ((((status >> 14) & 0x01) > 0)) statusList << "15";
    if ((((status >> 15) & 0x01) > 0)) statusList << "16";
    //    if (statusList.size() > 0) statusList.insert(0, QString::number(status, 16));
    return statusList.join("|");
}
// 0:停止均衡;0x55:强制;0xAA:自动;0x88:手动

enum BALANCE_MODE {
    BALANCE_STOP = 0x0,
    BALANCE_FORCE = 0x55,
    BALANCE_MANUAL = 0x88,
    BALANCE_AUTO = 0xAA,
};
QString mb_cmu::GetBalanceValue(uint16_t status) {
    int mode = (status & 0xFF);
    double Ib = (int8_t)((status >> 8) & 0xFF);
    Ib *= 0.1;
    switch (mode) {
        case BALANCE_STOP:
            return "STOP";
            break;
        case BALANCE_FORCE:
            return QString("FORCE:%1 A").arg(QString::number(Ib));
            break;
        case BALANCE_AUTO:
            return QString("AUTO:%1 A").arg(QString::number(Ib));
            break;
        case BALANCE_MANUAL:
            return QString("MANUAL:%1 A").arg(QString::number(Ib));
            break;
        default:
            break;
    }
    return QString("ERR:%1").arg(QString::number(mode));
}
void mb_cmu::Dump2CsvTitle() {
    if (stopDump) return;
    if ((rec & CSV) != CSV) return;
    fileTime = QDateTime::currentDateTime();
    QString fileName = fileTime.toString("yyyyMMdd_hhmmss");
    fileName.append(".csv");
    if (csvfile) csvfile->close();
    csvfile = new QFile(dataPath + "/" + fileName);
    if (!csvfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        delete csvfile;
        csvfile = nullptr;
        qDebug() << "Cannot open file for writing: " << qPrintable(csvfile->errorString());
        return;
    }
    QTextStream data_buf(csvfile);
    // 写入UTF-BOM头部
    data_buf << QChar(0xfeff);
    data_buf << "Time,";
    for (int i = 0; i < node_table_size; i++) {
        if (node_table[i].val_type == 128) {
            data_buf << (QString(node_table[i].name)) << ",";
        }
    }
    for (int i = 0; i < config.bmu_num; i++) {
        for (int j = 0; j < config.vol_num; ++j) {
            data_buf << (QString("BMU%1_U%2,").arg(i + 1).arg(j + 1));
        }
        for (int j = 0; j < config.T_num; j++) {
            data_buf << (QString("BMU%1_T%2,").arg(i + 1).arg(j + 1));
        }
        for (int j = 0; j < config.Tp_num; j++) {
            data_buf << (QString("BMU%1_Tp%2,").arg(i + 1).arg(j + 1));
        }
        data_buf << (QString("BMU%1_电压断线,").arg(i + 1));
        data_buf << (QString("BMU%1_温度断线,").arg(i + 1));
        data_buf << (QString("BMU%1_运行状态,").arg(i + 1));
        data_buf << (QString("BMU%1_故障状态,").arg(i + 1));
        if (protocal_ver > CMUV3) {
            data_buf << (QString("BMU%1_母线电压,").arg(i + 1));
            data_buf << (QString("BMU%1_均衡电流,").arg(i + 1));
            data_buf << (QString("BMU%1_均衡故障,").arg(i + 1));
            data_buf << (QString("BMU%1_通道状态,").arg(i + 1));
            data_buf << (QString("BMU%1_均衡模式,").arg(i + 1));
            data_buf << (QString("BMU%1_CAN错误,").arg(i + 1));
            for (int j = 0; j < config.vol_num; ++j) {
                data_buf << (QString("BMU%1_%2充电Ah,").arg(i + 1).arg(j + 1));
                data_buf << (QString("BMU%1_%2放电Ah,").arg(i + 1).arg(j + 1));
            }
        }
    }

    data_buf << endl;
}
#define FILE_ROTATE_TIME 60 * 60 * 12
void mb_cmu::Dump2Csv() {
    if (stopDump) return;
    if (rec & 0x01) {
        if (!csvfile) {
            Dump2CsvTitle();
        } else {
            // 检查csv文件以便分割文件 12小时
            if (fileTime.secsTo(QDateTime::currentDateTime()) >= FILE_ROTATE_TIME) {
                Dump2CsvTitle();
            }
        }
        QTextStream data_buf(csvfile);
        data_buf << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << ",";
        for (int i = 0; i < node_table_size; i++) {
            if (node_table[i].val_type == 128) {
                // QString("%1,").arg();
                data_buf << QString::number(this->tab_data.at(i).sysData.val.f64, 'g', 15) << ",";
            }
        }
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < config.vol_num; j++) {
                double val = this->bmu_data[i].Ucell[j] / 10000.0;
                data_buf << (QString("%1,").arg(val));
            }
            for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
                double val = this->bmu_data[i].Tcell[j] / 10.0;
                data_buf << (QString("%1,").arg(val));
            }

            // 20220115添加
            //状态量个数，电压断线+温度断线+运行状态+故障状态
            double val = this->bmu_data[i].Ubreak;
            data_buf << (QString("%1,").arg(val));
            val = this->bmu_data[i].Tbreak;
            data_buf << (QString("%1,").arg(val));
            val = this->bmu_data[i].RunStat;
            data_buf << (QString("%1,").arg(val));
            val = this->bmu_data[i].ErrStat;
            data_buf << (QString("%1,").arg(val));

            if (protocal_ver > CMUV3) {
                val = this->bmu_data[i].BalU24 / 1000.0;
                data_buf << (QString("%1,").arg(val));
                val = this->bmu_data[i].BalIdc / 1000.0;
                data_buf << (QString("%1,").arg(val));
                data_buf << GetBalanceStatus(this->bmu_data[i].BalErr) << ",";
                data_buf << GetBalanceStatus(this->bmu_data[i].BalStat) << ",";
                data_buf << GetBalanceValue(this->bmu_data[i].BalMode) << ",";
                data_buf << (this->bmu_data[i].CanErr) << ",";
                for (int j = 0; j < config.vol_num; j++) {
                    data_buf << (this->bmu_data[i].BalChgAh[j]) << ",";
                    data_buf << (this->bmu_data[i].BalDischgAh[j]) << ",";
                }
            }
        }
        data_buf << endl;
        csvfile->flush();
    }

    // dump bin文件
    if (rec & 0x02) {
        QJsonObject object;
        for (int i = 0; i < node_table_size; i++) {
            if (node_table[i].val_type == 128) {
                object.insert(QString(node_table[i].name), this->tab_data.at(i).sysData.val.f64);
            }
        }
        double val = 0;
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < config.vol_num; j++) {
                val = this->bmu_data[i].Ucell[j] / 10000.0;
                object.insert((QString("BMU%1_U%2").arg(i + 1).arg(j + 1)), val);
            }

            for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
                double val = this->bmu_data[i].Tcell[j] / 10.0;
                if (j < config.T_num) {
                    object.insert((QString("BMU%1_T%2").arg(i + 1).arg(j + 1)), val);
                } else {
                    object.insert((QString("BMU%1_Tp%2").arg(i + 1).arg(j + 1)), val);
                }
            }
            val = this->bmu_data[i].Ubreak;
            object.insert((QString("BMU%1_Ubreak,").arg(i + 1)), val);
            val = this->bmu_data[i].Tbreak;
            object.insert((QString("BMU%1_Tbreak,").arg(i + 1)), val);
            val = this->bmu_data[i].RunStat;
            object.insert((QString("BMU%1_Run,").arg(i + 1)), val);
            val = this->bmu_data[i].ErrStat;
            object.insert((QString("BMU%1_Err,").arg(i + 1)), val);
            //            if (protocal_ver > CMUV3) {
            //                val = this->bmu_data[i].BalU24;
            //                object.insert((QString("BMU%1_BalU24,").arg(i + 1)), val);
            //                val = this->bmu_data[i].BalIdc / 1000.0;
            //                object.insert((QString("BMU%1_BalIdc,").arg(i + 1)), val);
            //                object.insert((QString("BMU%1_BalErr,").arg(i + 1)),
            //                GetBalanceStatus(this->bmu_data[i].BalErr));
            //                object.insert((QString("BMU%1_BalStat,").arg(i + 1)),
            //                GetBalanceStatus(this->bmu_data[i].BalStat));
            //                object.insert((QString("BMU%1_BalMode,").arg(i + 1)),
            //                GetBalanceValue(this->bmu_data[i].BalMode));
            //            }
            //            object.insert((QString("BMU%1_Ver").arg(i + 1)),
            //            myHelper::IntegerToHexString(this->bmu_data[i].Version));
        }
        //        object.insert(QString("CMU_Ver"), myHelper::IntegerToHexString(cmu_ver));

        // 以读写方式打开主目录下的1.json文件，若该文件不存在则会自动创建
        QFile file(recPath + "/" + QDateTime::currentDateTime().toString("yyyyMMddThhmmss") + ".rec");
        if (!file.open(QIODevice::ReadWrite)) {
            qDebug() << "File open error";
        } /* else {
             qDebug() << "File open!";
         }*/
        // 使用QJsonDocument设置该json对象
        QJsonDocument jsonDoc;
        jsonDoc.setObject(object);
        // 将json以文本形式写入文件并关闭文件。
        QByteArray b = gzipCompress(jsonDoc.toBinaryData());
        file.write(b);
        file.close();
    }
}
mb_cmu::~mb_cmu() {
    if (this->cmu) this->Close();
    if (csvfile) csvfile->close();
    csvfile = nullptr;
    stop = true;
}

int mb_cmu::Close() {
    modbus_close(this->cmu);
    modbus_free(this->cmu);
    this->cmu = nullptr;
    return 0;
}

int mb_cmu::Init() {
    qDebug() << "init config";
    NodeReg node_reg_tmp;
    reg_list_.clear();
    wr_list_.clear();
    int index = -1;
    ST_NODE_DATA tmp_data;
    tmp_data.sysData.val.f64 = 0;
    tab_data.clear();
    tab_data.reserve(1000);
    config = {0, 0, 0, 0, 0};
    memset(&sys_para, 0, sizeof(sys_para));
    if (protocal_ver == CMUV2) {
        this->node_table = cmu_v2_config;
        this->node_table_size = cmu_v2_config_len;
    } else if (protocal_ver == CMUV3) {
        this->node_table = cmu_v3_config;
        this->node_table_size = cmu_v3_config_len;
    } else if (protocal_ver == CMUV4) {
        this->node_table = cmu_v4_config;
        this->node_table_size = cmu_v4_config_len;
    } else if (protocal_ver == CMUV4_1) {
        this->node_table = cmu_v4_1_config;
        this->node_table_size = cmu_v4_1_config_len;
    } else {
        this->node_table = cmu_v1_config;
        this->node_table_size = cmu_v1_config_len;
    }
    name_map.clear();
    for (int i = 0; i < node_table_size; i++) {
        node_reg_tmp.default_val = 0;
        if (node_table[i].reg_type > NONE_REG) {
            node_reg_tmp.reg_type = node_table[i].reg_type;
            node_reg_tmp.reg_addr = node_table[i].reg_addr;
            node_reg_tmp.data_type = node_table[i].data_type;
            node_reg_tmp.index = node_table[i].index;
            node_reg_tmp.factor = node_table[i].factor;
            if ((index = JudgeReg(node_reg_tmp)) != -1) {
                InsertReg(node_reg_tmp, index);
            } else {
                NewReg(node_reg_tmp);
            }
        }
        tab_data.push_back(tmp_data);
        name_map[node_table[i].name] = node_reg_tmp;
    }
    TMsgData MsgCmd;
    MsgCmd.msg_type = 0;
    MsgCmd.data.append((char*)&config, sizeof(config));
    pMq->sendMsg(99, MsgCmd);
    return 0;
}

#define MAX_LEN 125
/*
 * 读取数据
 **/
int mb_cmu::ReadData(uint8_t type, int start_addr, int reg_num, uint16_t* dest) {
    int status = 0;
    int read_len = 0;
    int rc = 0;
    if (!reg_num) return status;
    switch (type) {
        case MODBUS_FC_READ_HOLDING_REGISTERS: {
            do {
                read_len = reg_num > MAX_LEN ? MAX_LEN : reg_num;
                rc = modbus_read_registers(this->cmu, start_addr, read_len, dest);
                if (rc > 0) {
                    status += rc;
                } else {
                    qWarning() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc;
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        case MODBUS_FC_READ_INPUT_REGISTERS: {
            do {
                read_len = reg_num > MAX_LEN ? MAX_LEN : reg_num;
                rc = modbus_read_input_registers(this->cmu, start_addr, read_len, dest);
                if (rc > 0) {
                    status += rc;
                } else {
                    qWarning() << type << ",start:" << start_addr << ",len:" << read_len << ",rc:" << rc;
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        default:
            break;
    }
    if (status <= 0) {
        drv_status &= ~(0x01U << CMU_ONLINE);
        err_counter++;
    } else {
        err_counter = 0;
        drv_status |= (0x01 << CMU_ONLINE);
    }

    return status;
}
int mb_cmu::ReadCapData() {
    /* Read 5 registers from the address 0 */
    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    if (config.bmu_num > 0) {
        // BMU通信丢包计数
        reg_num = config.bmu_num * 2 * config.vol_num;
        status += ReadData(0x03, 0xA00 + config.bmu_num * 1, reg_num, p);
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < config.vol_num; j++) {
                bmu_data[i].BalChgAh[j] = *(p + i * 2 * config.vol_num + 2 * j);
                bmu_data[i].BalDischgAh[j] = *(p + i * 2 * config.vol_num + 2 * j + 1);
            }
        }
    }
    return status;
}
// CMU4.0主动均衡版本
#define STAT_NUM    4
#define BALANCE_NUM 5
int mb_cmu::ReadALL() {
    /* Read 5 registers from the address 0 */
    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    status += ReadAI();
    //
    if (config.bmu_num > 0) {
        reg_num = config.bmu_num * config.vol_num;
        status += ReadData(0x04, 0x01, reg_num, p);
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < config.vol_num; j++) {
                bmu_data[i].Ucell[j] = *(p + i * config.vol_num + j);
            }
        }
        reg_num = config.bmu_num * (config.T_num + config.Tp_num);
        status += ReadData(0x04, 0x1000, reg_num, p);
        for (int i = 0; i < config.bmu_num; i++) {
            for (int j = 0; j < (config.T_num + config.Tp_num); j++) {
                bmu_data[i].Tcell[j] = *(p + i * (config.T_num + config.Tp_num) + j);
            }
        }
        reg_num = config.bmu_num * 4;
        status += ReadData(0x03, 0x100, reg_num, p);
        for (int i = 0; i < config.bmu_num; i++) {
            bmu_data[i].Ubreak = *(p + i);
            bmu_data[i].Tbreak = *(p + i + 1 * config.bmu_num);
            bmu_data[i].RunStat = *(p + i + 2 * config.bmu_num);
            bmu_data[i].ErrStat = *(p + i + 3 * config.bmu_num);
        }
        if (protocal_ver > CMUV3) {
            reg_num = config.bmu_num * 5;  // 均衡状态等
            status += ReadData(0x03, 0x900, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                bmu_data[i].BalIdc = *(p + i * BALANCE_NUM);
                bmu_data[i].BalU24 = *(p + i * BALANCE_NUM + 1);
                bmu_data[i].BalErr = *(p + i * BALANCE_NUM + 2);
                bmu_data[i].BalStat = *(p + i * BALANCE_NUM + 3);
                bmu_data[i].BalMode = *(p + i * BALANCE_NUM + 4);
            }
            reg_num = config.bmu_num * 1;
            status += ReadData(0x03, 0xA00, reg_num, p);
            for (int i = 0; i < config.bmu_num; i++) {
                bmu_data[i].CanErr = *(p + i);
            }
        }
    }
    //版本号
    reg_num = config.bmu_num * 2 + 2;
    status += ReadData(0x03, 0x500, reg_num, p);
    this->cmu_ver = *(uint32_t*)(p);
    for (int i = 0; i < config.bmu_num; i++) {
        bmu_data[i].Version = *(uint32_t*)(p + 2 + i * 2);
    }
    return status;
}
void mb_cmu::run() {
    qDebug() << time(nullptr);
    if (time(nullptr) > (myHelper::cvt_TIME(__DATE__) + TIME_OUTOFDATE)) {
        qDebug() << "timeout exit..";
        this->stop = true;
        drv_status |= (0x01 << CMU_OUTOFDATE);
    }
    int rc = -1;
    TMsgData MsgCmd;
    uint32_t counter = 0;
    Init();
    emit signal_message("init complete.");
    while (1) {
        if (this->stop) break;
        while (pMq->readMsg(0, MsgCmd)) {
            qDebug() << "recv:" << MsgCmd.msg_type << ",len:" << MsgCmd.data.size() << "," << MsgCmd.data.toHex();
            DealCMD(MsgCmd);
        }
        //状态机
        if (err_counter++ >= 10) {
            qDebug() << "reconnect..." << time(NULL);
            err_counter = 0;
            state = SM_CONNECT;
        }
        switch (state) {
            case SM_READ:
                if (ReadALL()) {
                    if (protocal_ver > CMUV3) {
                        if (counter % (60 * 5) == 0) {
                            ReadCapData();
                        }
                        counter++;
//                        qDebug() << "counter" << counter;
                    }
                    state = SM_INIT;
                    Dump2Csv();
                }
                break;
            case SM_CONNECT: {
                drv_status &= ~(0x01U << CMU_ONLINE);
                if (cmu != nullptr) this->Close();
                cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
                modbus_set_slave(cmu, 1);
                modbus_set_response_timeout(cmu, 3, 0);
                if (cmu) rc = modbus_connect(this->cmu);
                if (rc == 0) state = SM_INIT;
                memset(tab_reg, 0, sizeof(tab_reg));
                qDebug() << "ip:" << this->mb_ip.c_str() << "port:" << this->mb_port;
                counter = 0;
                break;
            }
            case SM_INIT: {
                rc = ReadData(0x03, 5411, sizeof(sys_para) / 2, sys_para.array);
                if (rc == sizeof(sys_para) / 2) {
                    state = SM_READ;
                    sys_para.Name.u32LocalIP = bswap_32(sys_para.Name.u32LocalIP);
                    sys_para.Name.u32TftpServIP = bswap_32(sys_para.Name.u32TftpServIP);
                    isWrLocked = (sys_para.Name.uFunCtrReg & (0x01 << WR_LOCK_BIT)) > 0 ? true : false;
                    if (config.bmu_num != sys_para.Name.u16ClusterBmuNum ||
                        config.vol_num != sys_para.Name.u16BmuCellNum || config.T_num != sys_para.Name.u16BmuPackTNum ||
                        config.Tp_num != sys_para.Name.u16BmuPoleTNum) {
                        config.bmu_num = sys_para.Name.u16ClusterBmuNum;
                        config.vol_num = sys_para.Name.u16BmuCellNum;
                        config.T_num = sys_para.Name.u16BmuPackTNum;
                        config.Tp_num = sys_para.Name.u16BmuPoleTNum;
                        config.status_num = 4;
                        Dump2CsvTitle();
                        qDebug() << "table changed!";
                        TMsgData MsgCmd;
                        MsgCmd.msg_type = 0;
                        MsgCmd.data.append((char*)&config, sizeof(config));
                        pMq->sendMsg(99, MsgCmd);
                    }

                } else
                    sleep(1);
                break;
            }
            case SM_NONE:
            default:
                sleep(1);
                break;
        }
        usleep(500 * 1000);
    }
    qDebug() << "cmu exit..";
}

void mb_cmu::DealCMD(TMsgData& Msg) {
    int ret = -1;
    switch (Msg.msg_type) {
        case CONFIG_IP: {
            if (mb_ip != Msg.data.toStdString()) {
                mb_ip = Msg.data.toStdString();
                state = SM_CONNECT;
                qDebug() << "ip config:" << mb_ip.c_str();
            }
            return Msg.data.clear();
        } break;
        case CONFIG_PORT: {
            uint16_t port = 0;
            memcpy(&port, Msg.data.data(), sizeof(uint16_t));
            // if (mb_port == port) break;
            if (port != 0) {
                mb_port = port;
            }
            qDebug() << "port config:" << port;
            return Msg.data.clear();
        } break;
        case THREAD_EXIT:
            stop = true;
            qDebug() << "recv stop flag.";
            break;
        case CONFIG_INIT:
            drv_status = 0;
            state = SM_CONNECT;
            ret = 0;
            break;
        case CTRL_DO: {
            if (Msg.data.size() == 2 * sizeof(uint16_t)) {
                uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
                uint16_t addr = p[0];
                uint16_t value = p[1];
                ret = modbus_write_bit(cmu, addr, value);
            }
        } break;
        case CTRL_CMD_REBOOT: {
            uint16_t nb = Msg.data.size();
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            if (nb == 2 * sizeof(uint16_t)) {
                ret = write_ao(p[0], p[1]);
            }
            ret = 0;
            break;
        }

        case CTRL_AO: {
            uint16_t nb = Msg.data.size();
            if (nb < 2) break;
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            if (p[0] > node_table_size) break;
            if (nb == 2 * sizeof(uint16_t)) {
                uint16_t addr = node_table[p[0]].reg_addr;
                uint16_t value = p[1];
                ret = write_ao(addr, value);
            } else {
                uint16_t addr = node_table[p[0]].reg_addr;
                uint16_t* pv = (uint16_t*)&p[1];
                ret = write_ao(addr, (nb - 1) / 2, pv);
            }

            break;
        }
        case CTRL_SEC_AO: {
            uint16_t nb = Msg.data.size();
            if (nb != 2 * sizeof(uint16_t)) break;
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            ret = sec_ctrl(p[0], p[1]);
        } break;
        case CTRL_CMD_UNLOCK: {
            ret = write_ao(ADDR_WR_LOCK, MB_UNLOCK);
        } break;
        case CTRL_CMD_CLR_ALL_SOE: {
            ret = write_ao(ADDR_CLEAR_SOE, MB_CLR_ALL_SOE);
        } break;

        case CERT_CMD_TIME_ADJ: {
            uint32_t unix_time = static_cast<uint32_t>(time(nullptr));
            ret = write_ao(ADDR_TIME_ADJ, 2, (uint16_t*)(&unix_time));
        } break;
        case CERT_CMD_READ_SOE: {
            ret = ReadSOE();
            TMsgData MsgCmd;
            MsgCmd.data.clear();
            MsgCmd.msg_type = 1;
            MsgCmd.data.setNum(ret);
            pMq->sendMsg(99, MsgCmd);
        } break;
        case CTRL_AO_ADDR: {
            uint16_t nb = Msg.data.size();
            if (nb < 2) break;
            uint16_t* p = reinterpret_cast<uint16_t*>(Msg.data.data());
            if (nb == 2 * sizeof(uint16_t)) {
                ret = write_ao(p[0], p[1]);
            } else {
                uint16_t addr = p[0];
                uint16_t* pv = (uint16_t*)&p[1];
                ret = write_ao(addr, (nb - 1) / 2, pv);
            }
            break;
        }
        case CTRL_DUMP: {
            uint16_t nb = Msg.data.size();
            stopDump = (nb > 0);
            QSettings* settings = new QSettings("config.ini", QSettings::IniFormat);
            rec = settings->value("global/rec", "1").toInt();
            settings->deleteLater();
            qDebug() << "stop storage:" << stopDump;
            if (stopDump && csvfile) {
                qDebug() << "close old data file";
                csvfile->close();
                delete csvfile;
                csvfile = nullptr;
            }

            ret = 0;
        } break;
        case CTRL_SET_PRO: {
            uint16_t nb = Msg.data.size();
            if (nb == 1) {
                protocal_ver = BMS_PROTOCOL(Msg.data.toInt());
                qDebug() << "new cmu version:" << protocal_ver + 1;
                Init();
                Dump2CsvTitle();
            }
            ret = 0;
        } break;
        default:
            break;
    }
    Msg.data.clear();
    if (ret < 0) {
        qWarning() << tr("操作失败");
        emit signal_message(QString(tr("操作失败")));
    } else {
        qDebug() << tr("操作成功");
        emit signal_message(QString(tr("操作成功")));
    }
}
int mb_cmu::write_ao(uint16_t addr, uint16_t len, uint16_t* pv) {
    int ret = -1;
    if (!pv) return ret;
    if (len < 1) return ret;
    if (len == 1) {
        ret = write_ao(addr, *pv);
        return ret;
    }
    if (isWrLocked) modbus_write_register(cmu, ADDR_WR_LOCK, MB_UNLOCK);
    ret = modbus_write_registers(cmu, addr, len, pv);
    if (ret < 0)
        qWarning() << "wr aos failed" << addr << ":" << ret;
    else
        qDebug() << "wr aos " << addr << ":" << len;
    return ret;
}
int mb_cmu::write_ao(uint16_t addr, uint16_t v) {
    int ret = -1;
    if (isWrLocked) modbus_write_register(cmu, ADDR_WR_LOCK, MB_UNLOCK);
    ret = modbus_write_register(cmu, addr, v);
    if (ret < 0)
        qWarning() << "wr ao failed" << addr << ":" << ret;
    else
        qDebug() << "wr ao " << addr << ":" << v;
    return ret;
}
int mb_cmu::sec_ctrl(uint16_t addr, uint16_t type) {
    sec_cmd[8] = type;
    return write_ao(addr, 9, sec_cmd);
}
int mb_cmu::ParseData() {
    int ret = -1;

    return ret;
}
/**< 判别寄存器插入条件，返回可用报文序号，否则返回-1 */
int mb_cmu::JudgeReg(NodeReg& node_reg) {
    int index = reg_list_.size();
    const int max_frame_len = 229;
    int shortnum = (max_frame_len - 9) / 2;
    int bitnum = shortnum * 16;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) / 2;
    reg_len = (reg_len) > 1 ? reg_len : 1;
    uint8_t reg_type = node_reg.reg_type;
    while (index--) {
        DataReg data_reg = reg_list_.at(index);
        if (reg_type == DO_REG || reg_type == DI_REG) {
            if ((uint16_t)(node_reg.reg_addr - data_reg.reg_start) < bitnum && data_reg.reg_type == reg_type) {
                return index;
            }
        }
        if (reg_type == AO_REG || reg_type == AI_REG) {
            if ((uint16_t)(node_reg.reg_addr + reg_len - data_reg.reg_start) < shortnum &&
                data_reg.reg_type == reg_type) {
                return index;
            }
        }
    }
    return -1;
}
void mb_cmu::NewReg(NodeReg& node_reg) {
    DataReg data_reg;
    DatabaseIO data_io_tmp;
    data_reg.data_io.clear();
    data_reg.reg_type = node_reg.reg_type;
    data_reg.reg_start = node_reg.reg_addr;
    data_reg.data_num = 1;
    int reg_len = GET_RAWDATALEN(node_reg.data_type) >> 1;
    data_reg.reg_num = (reg_len) > 1 ? reg_len : 1;
    data_io_tmp.index = node_reg.index;
    data_io_tmp.factor = node_reg.factor;
    data_io_tmp.data_type = node_reg.data_type;
    data_io_tmp.offset = 0;
    data_reg.data_io.push_back(data_io_tmp);
    reg_list_.push_back(data_reg);
}
void mb_cmu::InsertReg(NodeReg& node_reg, int index) {
    try {
        DataReg* pdata_reg;
        pdata_reg = &reg_list_.at(index);
        DatabaseIO data_io_tmp;
        pdata_reg->data_num++;
        data_io_tmp.index = node_reg.index;
        data_io_tmp.factor = node_reg.factor;
        data_io_tmp.data_type = node_reg.data_type;
        data_io_tmp.offset = node_reg.reg_addr - pdata_reg->reg_start;
        int reg_len = GET_RAWDATALEN(node_reg.data_type) >> 1;
        reg_len = (reg_len) > 1 ? reg_len : 1;
        int len = data_io_tmp.offset + reg_len;
        if (pdata_reg->reg_num < len) pdata_reg->reg_num = len;
        pdata_reg->data_io.push_back(data_io_tmp);
    } catch (exception& e) {
        cout << e.what() << endl;
    }
}

int mb_cmu::ReadAI() {
    int res = -1;
    uint16_t tab_buf[128];
    for (vector<DataReg>::iterator iter = reg_list_.begin(); iter != reg_list_.end(); iter++) {
        res = ReadData(iter->reg_type, iter->reg_start, iter->reg_num, tab_buf);
        if (res == iter->reg_num) {
            for (vector<DatabaseIO>::iterator data_iter = iter->data_io.begin(); data_iter != iter->data_io.end();
                 data_iter++) {
                if (tab_data.size() > data_iter->index) {
                    tab_data.at(data_iter->index).isUpdate = true;
                    tab_data.at(data_iter->index).UpdateCnt++;
                    if (data_iter->data_type == 514) {
                        tab_data.at(data_iter->index).sysData.val.f64 = tab_buf[data_iter->offset] * data_iter->factor;
                    } else if (data_iter->data_type == 513) {
                        tab_data.at(data_iter->index).sysData.val.f64 =
                            (int16_t)tab_buf[data_iter->offset] * data_iter->factor;
                    } else if (data_iter->data_type == 17410) {
                        tab_data.at(data_iter->index).sysData.val.f64 =
                            MODBUS_GET_INT32_FROM_INT16_SWAP(tab_buf, data_iter->offset) * data_iter->factor;
                    }
                }
            }
        }
    }
    return res;
}
#define MAX_SOE_COUNT 500
#define SOE_REG_LEN   8

static uint16_t get_data(const uint16_t* src, int index) {
    uint16_t val = src[index];
    return static_cast<uint16_t>((val >> 8) | (val << 8));
}
int mb_cmu::ReadSOE() {
    int res = -1;
    uint16_t tab_buf[128] = {0};
    res = ReadData(0x03, 0x2000, 2, tab_buf);
    if (res != 2) return -1;
    memset(&cmu_soe, 0, sizeof(cmu_soe));
    cmu_soe.new_soe_count = tab_buf[0];
    cmu_soe.soe_count = tab_buf[1];
    int start = 0x2002;
    int len = cmu_soe.soe_count > MAX_SOE_COUNT ? MAX_SOE_COUNT : cmu_soe.soe_count;
    int soe_index = 0;
    do {
        int soe_len = len > 15 ? 15 : len;
        len -= soe_len;
        res = modbus_read_registers(cmu, start, soe_len * SOE_REG_LEN, tab_buf);
        // qDebug() << start << "->" << start + soe_len * SOE_REG_LEN<<","<<soe_index;
        if (res == soe_len * SOE_REG_LEN) {
            start += (soe_len * SOE_REG_LEN);
            for (int i = soe_len; --i >= 0;) {
                uint64_t u64time = static_cast<uint32_t>((get_data(tab_buf, SOE_REG_LEN * i + 1) << 16) |
                                                         get_data(tab_buf, SOE_REG_LEN * i));
                if (u64time > 0xFFFFFFFF) {
                    soe_index++;
                    continue;
                }
                u64time = u64time * 1000 + get_data(tab_buf, SOE_REG_LEN * i + 2);
                cmu_soe.list_soe[soe_index].soe_time = u64time;
                cmu_soe.list_soe[soe_index].soe_stat = get_data(tab_buf, SOE_REG_LEN * i + 3);
                cmu_soe.list_soe[soe_index].soe_type = get_data(tab_buf, SOE_REG_LEN * i + 4);
                cmu_soe.list_soe[soe_index].soe_id = get_data(tab_buf, SOE_REG_LEN * i + 5);
                cmu_soe.list_soe[soe_index].soe_val = get_data(tab_buf, SOE_REG_LEN * i + 6);
                cmu_soe.list_soe[soe_index].soe_limit = get_data(tab_buf, SOE_REG_LEN * i + 7);
                soe_index++;
            }
        } else {
            return -1;
        }
    } while (len);
    qDebug() << "read soe succeed.";
    return res;
}
