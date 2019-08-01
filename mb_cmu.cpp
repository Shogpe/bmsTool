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
    //memset(tab_config, 0, sizeof(tab_config));
    {
        static MB_CMD cmu_config[] = {
            {0x04, 0x01, uint16_t(config.bmu_num * config.vol_num), 0},
            {0x04, 4096, uint16_t(config.bmu_num * config.temp_num), 0},
            {0x03, 256, uint16_t(config.bmu_num * config.status_num), 0},
            {0x03, 1280, uint16_t(config.bmu_num * 2 + 2), 0},
            {0x03, 1, 12, 0},
            {0x03, 1024, 42, 0},
            {0x04, 5376, 25, 0},
            {0x03, 5376, 42, 0},
            {0x00, 0x00, 0, 0},
        };
        MB_CMD* pCmd = cmu_config;
        unsigned int offset = 0;
        for (; pCmd->type != 0; pCmd++) {
            pCmd->tab_offset = offset;
            offset += pCmd->reg_len;
            qDebug() << offset;
        }
        max_offset = offset;
        tab_config = (MB_CMD*)&cmu_config;
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
    uint16_t* p = this->tab_reg;
    int status = 0;
    MB_CMD* pCmd = tab_config;
    for (; pCmd->type != 0; pCmd++) {
        // qDebug()<<pCmd->type<<"len="<< pCmd->reg_len;
        reg_num += pCmd->reg_len;
        status += ReadData(pCmd->type, pCmd->start_addr, pCmd->reg_len, p + pCmd->tab_offset);
    }
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
    qDebug() << time(nullptr);
    if (time(nullptr) > (time_t)1577848139ULL) {
        qDebug() << "timeout exit..";
        this->stop = true;
        cmu_status |= (0x01 << CMU_OUTOFDATE);
    }
    while (1) {
        if (this->stop) break;
        this->Loop();
        sleep(1);
    }
    qDebug() << "cmu exit..";
}
