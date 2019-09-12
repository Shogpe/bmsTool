#include "mb_cmu.h"
#include <QDebug>
#include <QTimerEvent>
#include "utils.h"

mb_cmu::mb_cmu() {
    cmu = nullptr;
    cmu_status = 0;
    stop = false;
    mb_ip = "192.168.1.120";
    mb_port = 502;
    pMq = MessageQueue::getInstance();
    pMq->registMsgQueue(0);
    config = {0, 0, 0, 0};
}

mb_cmu::~mb_cmu() {
    // MessageQueue::deleteInstance();
    if (this->cmu) this->Close();
    stop = true;
}

int mb_cmu::Close() {
    modbus_close(this->cmu);
    modbus_free(this->cmu);
    this->cmu = nullptr;
    tab_config.clear();
    return 0;
}

int mb_cmu::Init() {
    int rc = -1;
    if (cmu != nullptr) this->Close();
    cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
    modbus_set_slave(cmu, 1);
    modbus_set_response_timeout(cmu, 3, 0);
    if (cmu) rc = modbus_connect(this->cmu);
    if (rc != 0) return rc;
    memset(tab_reg, 0, sizeof(tab_reg));
    ST_SysPara sys_para;
    rc = ReadData(0x03, 5376, TAB_CFG_LEN, sys_para.array);
    if (rc != TAB_CFG_LEN) return rc;
    config.bmu_num = sys_para.Name.u16ClusterBmuNum;
    config.vol_num = sys_para.Name.u16BmuCellNum;
    config.temp_num = sys_para.Name.u16BmuPackTNum + sys_para.Name.u16BmuPoleTNum;
    config.status_num = 4;
    tab_config.clear();
    qDebug() << "init config";
    {
        MB_CMD cmu_config[] = {
            {0x03, 5376, TAB_CFG_LEN, 0},
            {0x03, 1, TAB_SYS_LEN, 0},  //时钟,状态
            {0x03, 1024, TAB_ENG_LEN, 0},
            {0x04, 5376, TAB_CMU_LEN, 0},
            {0x04, 0x01, uint16_t(config.bmu_num * config.vol_num), 0},
            {0x04, 4096, uint16_t(config.bmu_num * config.temp_num), 0},
            {0x03, 256, uint16_t(config.bmu_num * config.status_num), 0},
            {0x03, 1280, uint16_t(config.bmu_num * 2 + 2), 0},
            {0x00, 0x00, 0, 0},
        };
        MB_CMD* pCmd = cmu_config;
        unsigned int offset = 0;
        for (; pCmd->type != 0; pCmd++) {
            pCmd->tab_offset = offset;
            offset += pCmd->reg_len;
            tab_config.push_back(*pCmd);
            qDebug() << offset;
        }
        max_offset = offset;
        qDebug() << "max len:" << this->max_offset << " size " << tab_config.size();
    }
    return 0;
}
#define MAX_LEN 100
/*
 * 读取数据
 **/
int mb_cmu::ReadData(uint8_t type, int start_addr, int reg_num, uint16_t* dest) {
    int status = 0;
    int read_len = 0;
    int rc = 0;
    switch (type) {
        case MODBUS_FC_READ_HOLDING_REGISTERS: {
            do {
                read_len = reg_num > MAX_LEN ? MAX_LEN : reg_num;
                rc = modbus_read_registers(this->cmu, start_addr, read_len, dest);
                if (rc > 0) {
                    status += rc;
                } else {
                    qDebug() << type << ",err:" << start_addr << ",len:" << read_len;
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
                    qDebug() << type << ",err:" << start_addr << ",len:" << read_len;
                }
                reg_num -= read_len;
                dest += read_len;
                start_addr += read_len;
            } while (reg_num);
        } break;
        default:
            break;
    }
    return status;
}
int mb_cmu::ReadALL() {
    /* Read 5 registers from the address 0 */
    static int err_counter = 0;
    int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;

    for (vector<MB_CMD>::iterator iter = tab_config.begin(); iter != tab_config.end(); iter++) {
        reg_num += iter->reg_len;
        status += ReadData(iter->type, iter->start_addr, iter->reg_len, p + iter->tab_offset);
    }
    //    for (int i=0; i< != 0; pCmd++) {
    //        // qDebug()<<pCmd->type<<"len="<< pCmd->reg_len;
    //        reg_num += pCmd->reg_len;
    //        status += ReadData(pCmd->type, pCmd->start_addr, pCmd->reg_len, p + pCmd->tab_offset);
    //    }
    if (reg_num != status) qDebug() << status << " should be " << reg_num;
    if (status <= 0) {
        cmu_status &= ~(0x01U << CMU_ONLINE);
        if (err_counter++ >= 5) {
            qDebug() << "reconnect...";
            err_counter = 0;
            modbus_close(this->cmu);
            qDebug() << time(NULL);
            sleep(5);
            modbus_connect(this->cmu);
        }
    } else {
        err_counter = 0;
        cmu_status |= (0x01 << CMU_ONLINE);
        return 1;
    }

    return 0;
}
#define TIME_OUTOFDATE 3 * 31 * 24 * 60 * 60
typedef enum {
    SM_NONE = 0,
    SM_CONNECT,  //
    SM_READ,     //
    SM_CTRL,     //
} STATE_MACHINE;
void mb_cmu::run() {
    qDebug() << time(nullptr);
    if (time(nullptr) > (cvt_TIME(__DATE__) + TIME_OUTOFDATE)) {
        qDebug() << "timeout exit..";
        this->stop = true;
        cmu_status |= (0x01 << CMU_OUTOFDATE);
    }
    int rc = -1;
    TMsgData MsgCmd;
    STATE_MACHINE state = SM_CONNECT;
    // modbus_set_debug(cmu, 1);
    while (1) {
        switch (state) {
            case SM_READ:
                ReadALL();
                break;
            case SM_CONNECT: {
                if (cmu != nullptr) this->Close();
                cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
                modbus_set_slave(cmu, 1);
                modbus_set_response_timeout(cmu, 3, 0);
                if (cmu) rc = modbus_connect(this->cmu);
                if (rc == 0) state = SM_READ;
                memset(tab_reg, 0, sizeof(tab_reg));
                tab_config.clear();
            }
            case SM_NONE:
            default:
                sleep(1);
                break;
        }
        if (this->stop) break;
        while (pMq->readMsg(0, MsgCmd)) {
            qDebug() << "recv " << MsgCmd.msg_type << "," << MsgCmd.data;
        }
        usleep(500 * 1000);
    }
    qDebug() << "cmu exit..";
}
void mb_cmu::DealCMD(TMsgData& Msg) {
    int ret = -1;
    switch (Msg.msg_type) {
        case CONFIG_IP: {
            mb_ip = Msg.data.toStdString();
        } break;
        case CONFIG_PORT: {
            mb_port = Msg.data.toInt();
            if (mb_port < 0 || mb_port > 65535) {
                mb_port = 502;
            }
        } break;
        case THREAD_EXIT:
            stop = true;
            break;
        case CONFIG_INIT:
            cmu_status = 0;
            Init();
            break;
        case CTRL_DO: {
            if (Msg.data.size() == 2 * sizeof(int)) {
                int* p = (int*)Msg.data.data();
                int addr = p[0];
                int value = p[1];
                ret = modbus_write_bit(cmu, addr, value);
                if(ret != 0) qDebug()<<"wr do failed" << ret;
            }
        } break;
        case CTRL_AO: {
            int nb = Msg.data.size();
            if (nb < 2) break;
            int* p = (int*)Msg.data.data();
            if (nb == 2 * sizeof(int)) {
                int addr = p[0];
                int value = p[1];
                ret = modbus_write_register(cmu, addr, value);
                if(ret != 0) qDebug()<<"wr ao failed" << ret;
            } else {
                int addr = p[0];
                uint16_t* pv = (uint16_t*)&p[1];
                ret = modbus_write_registers(cmu, addr, (nb - 1) / 2, pv);
                if(ret != 0) qDebug()<<"wr aos failed" << ret;
            }
            break;
        }
        default:
            break;
    }
}
