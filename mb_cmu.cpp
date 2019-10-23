#include "mb_cmu.h"
#include <QDebug>
#include <QTimerEvent>
#include "utils.h"
#include "myhelper.h"
static uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, WORD(0x9000), 0x1122, 0x3344, 0x5566};
static MB_NODE tab_config[] = {
    {0, "Umax", 4, 5376, 514, 128, 0.0001},
    {1, "UmaxID", 4, 5377, 514, 128, 1},
    {2, "Umin", 4, 5378, 514, 128, 0.0001},
    {3, "UminID", 4, 5379, 514, 128, 1},
    {4, "Tmax", 4, 5380, 513, 128, 0.1},
    {5, "TmaxID", 4, 5381, 514, 128, 1},
    {6, "Tmin", 4, 5382, 513, 128, 0.1},
    {7, "TminID", 4, 5383, 514, 128, 1},
    {8, "TpMax", 4, 5384, 513, 128, 0.1},
    {9, "TpMaxID", 4, 5385, 514, 128, 1},
    {10, "UdMax", 4, 5386, 513, 128, 0.001},
    {11, "UdMaxID", 4, 5387, 514, 128, 0.1},
    {12, "TrMax", 4, 5388, 514, 128, 0.1},
    {13, "TrMaxID", 4, 5389, 514, 128, 1},
    {14, "UmMax", 4, 5390, 514, 128, 0.001},
    {15, "UmMaxID", 4, 5391, 514, 128, 1},
    {16, "Udc", 4, 5392, 514, 128, 0.1},
    {17, "RIns", 4, 5393, 514, 128, 0.1},
    {18, "RpIns", 4, 5394, 514, 128, 0.1},
    {19, "RnIns", 4, 5395, 514, 128, 0.1},
    {20, "ILeak", 4, 5396, 513, 128, 0.1},
    {21, "Idc", 4, 5397, 513, 128, 0.1},
    {22, "TExt", 4, 5398, 513, 128, 0.1},
    {23, "TpExt", 4, 5399, 513, 128, 0.1},
    {24, "TnExt", 4, 5400, 513, 128, 0.1},
    {25, "sysTime", 3, 1, 17410, 128, 1},
    {26, "sysStatus1", 3, 3, 514, 128, 1},
    {27, "sysStatus2", 3, 4, 514, 128, 1},
    {28, "sysErrStatus", 3, 5, 514, 128, 1},
    {29, "sysAlmStatus", 3, 6, 514, 128, 1},
    {30, "sysComm1", 3, 7, 1028, 128, 1},
    {31, "sysComm2", 3, 9, 514, 128, 1},
    {32, "sysDOStatus", 3, 11, 514, 128, 1},
    {33, "sysDIStatus", 3, 12, 514, 128, 1},
    {34, "SOC", 3, 1024, 514, 128, 0.1},
    {35, "SOH", 3, 1025, 514, 128, 0.1},
    {36, "Pdc", 3, 1026, 513, 128, 0.1},
    {37, "ERemain", 3, 1027, 17410, 128, 0.1},
    {38, "ECharge", 3, 1029, 17410, 128, 0.1},
    {39, "EDischarge", 3, 1031, 17410, 128, 0.1},
    {40, "ECurCharge", 3, 1033, 17410, 128, 0.1},
    {41, "ECurDischarge", 3, 1035, 17410, 128, 0.1},
    {42, "当前剩余库伦", 3, 1037, 17410, 128, 0.1},
    {43, "当前输入库伦", 3, 1039, 17410, 128, 0.1},
    {44, "当前输出库伦", 3, 1041, 17410, 128, 0.1},
    {45, "累计输入库伦", 3, 1043, 17410, 128, 0.1},
    {46, "累计输出库伦", 3, 1045, 17410, 128, 0.1},
    {47, "TCharge", 3, 1047, 17410, 128, 1},
    {48, "TDischarge", 3, 1049, 17410, 128, 1},
    {49, "TCurCharge", 3, 1051, 17410, 128, 1},
    {50, "TCurDischarge", 3, 1053, 17410, 128, 1},
    {51, "CountCharge", 3, 1055, 17410, 128, 1},
    {52, "CountDischarge", 3, 1057, 17410, 128, 1},
    {53, "充电开始时间", 3, 1059, 17410, 128, 1},
    {54, "充电停止时间", 3, 1061, 17410, 128, 1},
    {55, "放电开始时间", 3, 1063, 17410, 128, 1},
    {56, "放电停止时间", 3, 1065, 17410, 128, 1},
    {57, "CellVolH", 3, 5376, 514, 128, 0.0001},
    {58, "CellVolHH", 3, 5377, 514, 128, 0.0001},
    {59, "CellVolL", 3, 5378, 514, 128, 0.0001},
    {60, "CellVolLL", 3, 5379, 514, 128, 0.0001},
    {61, "PackTH", 3, 5380, 513, 128, 0.1},
    {62, "PackTHH", 3, 5381, 513, 128, 0.1},
    {63, "PackTL", 3, 5382, 513, 128, 0.1},
    {64, "PackTLL", 3, 5383, 513, 128, 0.1},
    {65, "PackTdH", 3, 5384, 513, 128, 0.1},
    {66, "PackTdHH", 3, 5385, 513, 128, 0.1},
    {67, "PackTrH", 3, 5386, 513, 128, 0.1},
    {68, "PackTrHH", 3, 5387, 513, 128, 0.1},
    {69, "PoleTH", 3, 5388, 513, 128, 0.1},
    {70, "PoleTHH", 3, 5389, 513, 128, 0.1},
    {71, "ClusterCurH", 3, 5390, 514, 128, 1},
    {72, "ClusterCurHH", 3, 5391, 514, 128, 1},
    {73, "ClusterCurShort", 3, 5392, 514, 128, 1},
    {74, "ClusterVolH", 3, 5393, 514, 128, 0.1},
    {75, "ClusterVolHH", 3, 5394, 514, 128, 0.1},
    {76, "ClusterVolL", 3, 5395, 514, 128, 0.1},
    {77, "ClusterVolLL", 3, 5396, 514, 128, 0.1},
    {78, "ClusterRIns", 3, 5397, 514, 128, 1},
    {79, "ClusterCurLeak", 3, 5398, 514, 128, 0.1},
    {80, "ClusterTAlm", 3, 5399, 514, 128, 1},
    {81, "ClusterTErr", 3, 5400, 514, 128, 1},
    {82, "ClusterE", 3, 5401, 514, 128, 0.1},
    {83, "ClusterEAdj", 3, 5402, 514, 128, 0.1},
    {84, "ClusterEremain", 3, 5403, 514, 128, 0.1},
    {85, "ClusterIe", 3, 5404, 514, 128, 0.1},
    {86, "ClusterCurRange", 3, 5405, 514, 128, 1},
    {87, "ClusterILeakRg", 3, 5406, 514, 128, 1},
    {88, "ClusterVolRange", 3, 5407, 514, 128, 1},
    {89, "BalnceMask", 3, 5408, 514, 128, 1},
    {90, "BalnceStart", 3, 5409, 514, 128, 0.0001},
    {91, "BalnceStartDiff", 3, 5410, 514, 128, 0.1},
    {92, "ClusterBmuNum", 3, 5411, 514, 128, 1},
    {93, "BmuCellNum", 3, 5412, 514, 128, 1},
    {94, "BmuPackTNum", 3, 5413, 514, 128, 1},
    {95, "BmuPoleTNum", 3, 5414, 514, 128, 1},
    {96, "ClusterAlmMask", 3, 5415, 514, 128, 1},
    {97, "ClusterErrMask", 3, 5416, 514, 128, 1},
    {98, "FuncMask", 3, 5417, 514, 128, 1},
    {99, "IP", 3, 5418, 514, 128, 1},
    {100, "ServerIP", 3, 5420, 514, 128, 1},
};
#define MAX_CFG 101
mb_cmu::mb_cmu() {
    cmu = nullptr;
    cmu_status = 0;
    stop = false;
    mb_ip = "192.168.1.120";
    mb_port = 502;
    pMq = MessageQueue::getInstance();
    pMq->registMsgQueue(0);
    tab_data.reserve(1000);
    config = {0, 0, 0, 0, 0};
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

int mb_cmu::Init() {
    qDebug() << "init config";
    NodeReg node_reg_tmp;
    reg_list_.clear();
    wr_list_.clear();
    int index = -1;
    ST_NODE_DATA tmp_data;
    tmp_data.sysData.val.f64 = 0;
    for (int i = 0; i < MAX_CFG; i++) {
        node_reg_tmp.default_val = 0;
        if (tab_config[i].reg_type > NONE_REG) {
            node_reg_tmp.reg_type = tab_config[i].reg_type;
            node_reg_tmp.reg_addr = tab_config[i].reg_addr;
            node_reg_tmp.data_type = tab_config[i].data_type;
            node_reg_tmp.index = tab_config[i].index;
            node_reg_tmp.factor = tab_config[i].factor;
            if ((index = JudgeReg(node_reg_tmp)) != -1) {
                InsertReg(node_reg_tmp, index);
            } else {
                NewReg(node_reg_tmp);
            }
        }
        tab_data.push_back(tmp_data);
        name_map[tab_config[i].name] = node_reg_tmp;
    }
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
    if (status <= 0) {
        cmu_status &= ~(0x01U << CMU_ONLINE);
        err_counter++;
    } else {
        err_counter = 0;
        cmu_status |= (0x01 << CMU_ONLINE);
    }

    return status;
}
int mb_cmu::ReadALL() {
    /* Read 5 registers from the address 0 */

    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    unsigned int offset = 0;
    status += ReadAI();
    //
    if (config.bmu_num > 0) {
        reg_num = config.bmu_num * config.vol_num;
        status += ReadData(0x04, 0x01, reg_num, p + offset);
        offset += reg_num;
        reg_num = config.bmu_num * (config.T_num + config.Tp_num);
        status += ReadData(0x04, 0x1000, reg_num, p + offset);
        offset += reg_num;
        reg_num = config.bmu_num * config.status_num;
        status += ReadData(0x03, 0x100, reg_num, p + offset);
        offset += reg_num;
        //版本号
        reg_num = config.bmu_num * 2 + 2;
        status += ReadData(0x03, 0x500, reg_num, p + offset);
        offset += reg_num;
    }

    return status;
}
#define TIME_OUTOFDATE 3 * 31 * 24 * 60 * 60

void mb_cmu::run() {
    qDebug() << time(nullptr);
    if (time(nullptr) > (myHelper::cvt_TIME(__DATE__) + TIME_OUTOFDATE)) {
        qDebug() << "timeout exit..";
        this->stop = true;
        cmu_status |= (0x01 << CMU_OUTOFDATE);
    }
    int rc = -1;
    TMsgData MsgCmd;

    Init();
    while (1) {
        if (this->stop) break;
        while (pMq->readMsg(0, MsgCmd)) {
            qDebug() << "recv " << MsgCmd.msg_type << "," << MsgCmd.data.toHex();
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
                    state = SM_INIT;
                }
                break;
            case SM_CONNECT: {
                cmu_status &= ~(0x01U << CMU_ONLINE);
                if (cmu != nullptr) this->Close();
                cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
                modbus_set_slave(cmu, 1);
                modbus_set_response_timeout(cmu, 3, 0);
                if (cmu) rc = modbus_connect(this->cmu);
                if (rc == 0) state = SM_INIT;
                memset(tab_reg, 0, sizeof(tab_reg));
                qDebug() << "ip:" << this->mb_ip.c_str() << "port:" << this->mb_port;
                break;
            }
            case SM_INIT: {
                ST_SysPara sys_para;
                rc = ReadData(0x03, 0x1500, TAB_CFG_LEN, sys_para.array);
                if (rc == TAB_CFG_LEN) {
                    state = SM_READ;
                    if (config.bmu_num != sys_para.Name.u16ClusterBmuNum ||
                        config.vol_num != sys_para.Name.u16BmuCellNum || config.T_num != sys_para.Name.u16BmuPackTNum ||
                        config.Tp_num != sys_para.Name.u16BmuPoleTNum) {
                        config.bmu_num = sys_para.Name.u16ClusterBmuNum;
                        config.vol_num = sys_para.Name.u16BmuCellNum;
                        config.T_num = sys_para.Name.u16BmuPackTNum;
                        config.Tp_num = sys_para.Name.u16BmuPoleTNum;
                        config.status_num = 4;

                        TMsgData MsgCmd;
                        MsgCmd.msg_type = 0;
                        MsgCmd.data.resize(sizeof(config));
                        memcpy(MsgCmd.data.data(), &config, sizeof(config));
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
        } break;
        case CONFIG_PORT: {
            uint16_t port = 0;
            memcpy(&port, Msg.data.data(), sizeof(uint16_t));
            if (mb_port == port) break;
            if (port != 0) {
                mb_port = port;
            }
            qDebug() << "port config:" << port;
        } break;
        case THREAD_EXIT:
            stop = true;
            qDebug() << "recv stop flag.";
            break;
        case CONFIG_INIT:
            cmu_status = 0;
            Init();
            break;
        case CTRL_DO: {
            if (Msg.data.size() == 2 * sizeof(uint16_t)) {
              uint16_t* p = reinterpret_cast< uint16_t *>(Msg.data.data());
                uint16_t addr = p[0];
                uint16_t value = p[1];
                ret = modbus_write_bit(cmu, addr, value);
                if (ret < 0) qDebug() << QString("wr do %1 failed(%2)").arg(addr).arg(ret);
            }
        } break;
        case CTRL_AO: {
            uint16_t nb = Msg.data.size();
            if (nb < 2) break;
            uint16_t* p = reinterpret_cast< uint16_t *>(Msg.data.data());
            if (p[0] > MAX_CFG) break;
            if (nb == 2 * sizeof(uint16_t)) {
                uint16_t addr = tab_config[p[0]].reg_addr;
                uint16_t value = p[1];
                ret = write_ao(addr, value);
            } else {
                uint16_t addr = tab_config[p[0]].reg_addr;
                uint16_t* pv = (uint16_t*)&p[1];
                ret = write_ao(addr, (nb - 1) / 2, pv);
            }
            break;
        }
        case CTRL_DOWN_BMS: {
            sec_ctrl(ADDR_UPGRADE, MB_UpdateCMU);
        } break;
        case CTRL_DOWN_BMS_BTL: {
            sec_ctrl(ADDR_UPGRADE, MB_UpdateBTC);
        } break;
        case CTRL_DOWN_BMU: {
            sec_ctrl(ADDR_UPGRADE, MB_UpdateBMU);
        } break;
        case CTRL_DOWN_BMU_BTL: {
            sec_ctrl(ADDR_UPGRADE, MB_UpdateBTB);
        } break;
        case CTRL_UPGRADE_BMU: {
            sec_ctrl(ADDR_UPGRADE, MB_UpdBmuNDL);
        } break;
        case CTRL_ADJ_U_FULL: {
            sec_ctrl(ADDR_ADJ, MB_Adj_VFull);
        } break;
        case CTRL_ADJ_U_ZERO: {
            sec_ctrl(ADDR_ADJ, MB_Adj_VZero);
        } break;
        case CTRL_ADJ_I_FULL: {
            sec_ctrl(ADDR_ADJ, MB_Adj_IFull);
        } break;
        case CTRL_ADJ_I_ZERO: {
            sec_ctrl(ADDR_ADJ, MB_Adj_IZero);
        } break;
        case CTRL_ADJ_ILEAK_FULL: {
            sec_ctrl(ADDR_ADJ, MB_Adj_LFull);
        } break;
        case CTRL_ADJ_ILEAK_ZERO: {
            sec_ctrl(ADDR_ADJ, MB_Adj_LZero);
        } break;
        case CTRL_ADJ_RINS_FULL: {
            sec_ctrl(ADDR_ADJ, MB_Adj_RFull);
        } break;
        case CTRL_ADJ_RINS_ZERO: {
            sec_ctrl(ADDR_ADJ, MB_Adj_RZero);
        } break;
        case CTRL_CMD_BMU_UNLOCK: {
            write_ao(ADDR_RESET_FACTORY, MB_BMU_UNLOCK);
            break;
        }
        case CTRL_CMD_BMU_LOCK: {
            write_ao(ADDR_RESET_FACTORY, MB_BMU_LOCK);
        } break;
        case CTRL_CMD_UNLOCK: {
            write_ao(ADDR_WR_LOCK, MB_UNLOCK);
        } break;
        case CTRL_CMD_RESET: {
            write_ao(ADDR_RESET_FACTORY, MB_FACTORY);
        } break;
        case CTRL_CMD_CLR_ENG: {
            write_ao(ADDR_CLEAR_ENG, MB_CLEAR_ENG);
        } break;
        case CTRL_CMD_CLR_ALL_SOE: {
            write_ao(ADDR_CLEAR_SOE, MB_CLR_ALL_SOE);
        } break;
        case CTRL_CMD_REBOOT: {
            write_ao(ADDR_REBOOT, MB_REBOOT);
        } break;
        case CERT_CMD_TIME_ADJ: {
            uint32_t unix_time = static_cast<uint32_t>(time(nullptr));
            write_ao(ADDR_TIME_ADJ, 2, (uint16_t*)(&unix_time));
        } break;
        case CERT_CMD_READ_SOE: {
            ReadSOE();
            TMsgData MsgCmd;
            MsgCmd.data.clear();
            MsgCmd.msg_type = 1;
            pMq->sendMsg(99, MsgCmd);
        } break;
        default:
            break;
    }
    Msg.data.clear();
}
int mb_cmu::write_ao(uint16_t addr, uint16_t len, uint16_t* pv) {
    int ret = -1;
    if (!pv) return ret;
    if (len < 1) return ret;
    if (len == 1) {
        ret = write_ao(addr, *pv);
        return ret;
    }
    ret = modbus_write_registers(cmu, addr, len, pv);
    if (ret < 0)
        qDebug() << "wr aos failed" << addr << ":" << ret;
    else
        qDebug() << "wr aos " << addr << ":" << len;
    return ret;
}
int mb_cmu::write_ao(uint16_t addr, uint16_t v) {
    int ret = -1;
    ret = modbus_write_register(cmu, addr, v);
    if (ret < 0)
        qDebug() << "wr ao failed" << addr << ":" << ret;
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
                            MODBUS_GET_INT32_FROM_INT16(tab_buf, data_iter->offset) * data_iter->factor;
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
    int len = MAX_SOE_COUNT;
    int soe_index = 0;
    do {
        int soe_len = len > 15 ? 15 : len;
        len -= soe_len;
        res = modbus_read_registers(cmu, start, soe_len * SOE_REG_LEN, tab_buf);
        //qDebug() << start << "->" << start + soe_len * SOE_REG_LEN<<","<<soe_index;
        if (res == soe_len * SOE_REG_LEN) {
            start += (soe_len * SOE_REG_LEN);
            for (int i = soe_len;--i >= 0;) {
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
