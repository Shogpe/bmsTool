#include "mb_cmu.h"

#include <QDebug>
#include <QTimerEvent>
mb_cmu::mb_cmu() {
    this->cmu = nullptr;
    this->cmu_status = 0;
    stop = false;
}

mb_cmu::~mb_cmu() {
    if (this->cmu) this->Close();
    stop = true;
}

int mb_cmu::Close() {
    modbus_close(this->cmu);
    modbus_free(this->cmu);
    this->cmu = nullptr;
    return 0;
}

int mb_cmu::Init(string ip, int port) {
    this->mb_ip = ip;
    this->mb_port = port;
    if (cmu != nullptr) this->Close();
    cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
    modbus_set_slave(cmu, 1);
    modbus_set_response_timeout(cmu, 3, 0);
    if (cmu) modbus_connect(this->cmu);
    memset(tab_reg, 0, sizeof(tab_reg));
    memset(tab_config, 0, sizeof(tab_config));
    {
        tab_config[0].type = 0x04;
        tab_config[0].start_addr = 0x01;
        tab_config[0].reg_len = config.bmu_num * config.vol_num;
        tab_config[0].tab_offset = 0;
        tab_config[1].type = 0x04;
        tab_config[1].start_addr = 4096;
        tab_config[1].reg_len = config.bmu_num * config.temp_num;
        tab_config[1].tab_offset = tab_config[0].reg_len + tab_config[0].tab_offset;
        tab_config[2].type = 0x03;
        tab_config[2].start_addr = 256;
        tab_config[2].reg_len = config.bmu_num * config.status_num;
        tab_config[2].tab_offset = tab_config[1].reg_len + tab_config[1].tab_offset;
        tab_config[3].type = 0x03;
        tab_config[3].start_addr = 1280;
        tab_config[3].reg_len = config.bmu_num * 2 + 2;
        tab_config[3].tab_offset = tab_config[2].reg_len + tab_config[2].tab_offset;
        //时钟
        tab_config[4].type = 0x03;
        tab_config[4].start_addr = 1;
        tab_config[4].reg_len = 12;
        tab_config[4].tab_offset = tab_config[3].reg_len + tab_config[3].tab_offset;
        //统计量
        tab_config[5].type = 0x03;
        tab_config[5].start_addr = 1024;
        tab_config[5].reg_len = 42;
        tab_config[5].tab_offset = tab_config[4].reg_len + tab_config[4].tab_offset;
        //极值
        tab_config[6].type = 0x04;
        tab_config[6].start_addr = 5376;
        tab_config[6].reg_len = 25;
        tab_config[6].tab_offset = tab_config[5].reg_len + tab_config[5].tab_offset;
        //设定值
        tab_config[7].type = 0x03;
        tab_config[7].start_addr = 5376;
        tab_config[7].reg_len = 42;
        tab_config[7].tab_offset = tab_config[6].reg_len + tab_config[6].tab_offset;
        //其余结束
        tab_config[8].type = 0x00;
        tab_config[8].start_addr = 0;
        tab_config[8].reg_len = 0;
        tab_config[8].tab_offset = tab_config[7].reg_len + tab_config[7].tab_offset;
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
int mb_cmu::Loop() {
    /* Read 5 registers from the address 0 */
    static int err_counter = 0;
    int reg_num = 0;
    int start_addr = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    MB_CMD* pCmd = tab_config;
    for (; pCmd->type != 0; pCmd++) {
        // qDebug()<<pCmd->type<<"len="<< pCmd->reg_len;
        reg_num += pCmd->reg_len;
        status += ReadData(pCmd->type, pCmd->start_addr, pCmd->reg_len, p + pCmd->tab_offset);
    }
    //    //单体电压
    //    status += ReadData(tab_config.volatge.type, tab_config.volatge.start_addr, tab_config.volatge.reg_len,
    //                       p + tab_config.volatge.tab_offset);
    //    //单体温度
    //    status += ReadData(tab_config[1].type, tab_config[1].start_addr, tab_config[1].reg_len,
    //                       p + tab_config[1].tab_offset);
    //    // BMU状态
    //    status += ReadData(tab_config[2].type, tab_config[2].start_addr, tab_config[2].reg_len,
    //                       p + tab_config[2].tab_offset);
    //    // CMU及BMU版本号
    //    status += ReadData(tab_config[3].type, tab_config[3].start_addr, tab_config[3].reg_len,
    //                       p + tab_config[3].tab_offset);
    if (reg_num != status) qDebug() << status << " should be " << reg_num;
    if (status <= 0) {
        cmu_status &= ~(0x01U << CMU_ONLINE);
        if (err_counter++ >= 5) {
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
void mb_cmu::run() {
    this->Init("192.168.1.125", 502);
    while (1) {
        this->Loop();
        if (this->stop) break;
        sleep(1);
    }
    qDebug() << "cmu exit..";
}
