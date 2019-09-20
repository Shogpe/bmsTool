#include "mb_cmu.h"
#include <QDebug>
#include <QTimerEvent>
#include "utils.h"
static uint16_t sec_cmd[9] = {0x1223, 0x3445, 0x5667, 0x7889, WORD(0x9000), 0x1122, 0x3344, 0x5566};
static MB_NODE tab_config[] = {
    {0, "单体电池电压最大值", 4, 5376, 514, 128, 0.0001f},
    {1, "电压最大单体电池编号", 4, 5377, 514, 128, 1},
    {2, "单体电池电压最小值", 4, 5378, 514, 128, 0.0001f},
    {3, "电压最小单体电池编号", 4, 5379, 514, 128, 1},
    {4, "电池模组温度最高值", 4, 5380, 513, 128, 0.1f},
    {5, "温度最高电池模组编号", 4, 5381, 514, 128, 1},
    {6, "电池模组温度最低值", 4, 5382, 513, 128, 0.1f},
    {7, "温度最低电池模组编号", 4, 5383, 514, 128, 1},
    {8, "Pack极柱温度最高值", 4, 5384, 513, 128, 0.1f},
    {9, "温度最高Pack编号", 4, 5385, 514, 128, 1},
    {10, "最大单体电压差值", 4, 5386, 513, 128, 0.001f},
    {11, "最大电池模组温差值", 4, 5387, 514, 128, 0.1f},
    {12, "最大电池模组温升值", 4, 5388, 514, 128, 0.1f},
    {13, "最大温升电池模组编号", 4, 5389, 514, 128, 1},
    {14, "最大模组电压值", 4, 5390, 514, 128, 0.001f},
    {15, "最大模组电压ID", 4, 5391, 514, 128, 1},
    {16, "簇电压", 4, 5392, 514, 128, 0.1f},
    {17, "簇绝缘电阻", 4, 5393, 514, 128, 0.1f},
    {18, "簇正母线绝缘电阻值", 4, 5394, 514, 128, 0.1f},
    {19, "簇负母线绝缘电阻值", 4, 5395, 514, 128, 0.1f},
    {20, "簇漏电流值", 4, 5396, 513, 128, 0.1f},
    {21, "簇总电流值", 4, 5397, 513, 128, 0.1f},
    {22, "簇外接温度值", 4, 5398, 513, 128, 0.1f},
    {23, "簇正极柱温度值", 4, 5399, 513, 128, 0.1f},
    {24, "簇负极柱温度值", 4, 5400, 513, 128, 0.1f},
    {25, "时钟", 3, 1, 17410, 128, 1},
    {26, "nBC系统状态寄存器1", 3, 3, 514, 128, 1},
    {27, "nBC系统状态寄存器2", 3, 4, 514, 128, 1},
    {28, "nBC保护状态位", 3, 5, 514, 128, 1},
    {29, "nBC报警状态位", 3, 6, 514, 128, 1},
    {30, "nBMU通讯状态1", 3, 7, 1028, 128, 1},
    {31, "nBMU通讯状态2", 3, 9, 514, 128, 1},
    {32, "nBMS_DO状态", 3, 11, 514, 128, 1},
    {33, "nBMS_DI状态", 3, 12, 514, 128, 1},
    {34, "SOC", 3, 1024, 514, 128, 0.1f},
    {35, "SOH", 3, 1025, 514, 128, 0.1f},
    {36, "直流功率", 3, 1026, 513, 128, 0.1f},
    {37, "当前剩余电量", 3, 1027, 17410, 128, 0.1f},
    {38, "累计充电量", 3, 1029, 17410, 128, 0.1f},
    {39, "累计放电量", 3, 1031, 17410, 128, 0.1f},
    {40, "当前充电电量", 3, 1033, 17410, 128, 0.1f},
    {41, "当前放电电量", 3, 1035, 17410, 128, 0.1f},
    {42, "当前剩余库伦", 3, 1037, 17410, 128, 0.1f},
    {43, "当前输入库伦", 3, 1039, 17410, 128, 0.1f},
    {44, "当前输出库伦", 3, 1041, 17410, 128, 0.1f},
    {45, "累计输入库伦", 3, 1043, 17410, 128, 0.1f},
    {46, "累计输出库伦", 3, 1045, 17410, 128, 0.1f},
    {47, "累计充电时间", 3, 1047, 17410, 128, 1},
    {48, "累计放电时间", 3, 1049, 17410, 128, 1},
    {49, "当前充电时间", 3, 1051, 17410, 128, 1},
    {50, "当前放电时间", 3, 1053, 17410, 128, 1},
    {51, "累计充电次数", 3, 1055, 17410, 128, 1},
    {52, "累计放电次数", 3, 1057, 17410, 128, 1},
    {53, "充电开始时间", 3, 1059, 17410, 128, 1},
    {54, "充电停止时间", 3, 1061, 17410, 128, 1},
    {55, "放电开始时间", 3, 1063, 17410, 128, 1},
    {56, "放电停止时间", 3, 1065, 17410, 128, 1},
    {57, "单体电池电压越上限告警值", 3, 5376, 514, 128, 0.0001f},
    {58, "单体电池电压越上上限保护值", 3, 5377, 514, 128, 0.0001f},
    {59, "单体电池电压越下限告警值", 3, 5378, 514, 128, 0.0001f},
    {60, "单体电池电压越下下限保护值", 3, 5379, 514, 128, 0.0001f},
    {61, "单体电池温度越上限告警值", 3, 5380, 513, 128, 0.1f},
    {62, "单体电池温度越上上限保护值", 3, 5381, 513, 128, 0.1f},
    {63, "单体电池温度越下限告警值", 3, 5382, 513, 128, 0.1f},
    {64, "单体电池温度越下下限保护值", 3, 5383, 513, 128, 0.1f},
    {65, "单体电池温差越上限告警值", 3, 5384, 513, 128, 0.1f},
    {66, "单体电池温差越上上限保护值", 3, 5385, 513, 128, 0.1f},
    {67, "单体电池温升越上限告警值", 3, 5386, 513, 128, 0.1f},
    {68, "单体电池温升越上上限保护值", 3, 5387, 513, 128, 0.1f},
    {69, "Pack极柱温度越上限告警值", 3, 5388, 513, 128, 0.1f},
    {70, "Pack极柱温度越上上限保护值", 3, 5389, 513, 128, 0.1f},
    {71, "充放电电流越上限过负荷告警值", 3, 5390, 514, 128, 1},
    {72, "充放电电流越上上限过负荷保护值", 3, 5391, 514, 128, 1},
    {73, "短路电流保护值", 3, 5392, 514, 128, 1},
    {74, "簇电压越上限告警值", 3, 5393, 514, 128, 0.1f},
    {75, "簇电压越上上限保护值", 3, 5394, 514, 128, 0.1f},
    {76, "簇电压越下限告警值", 3, 5395, 514, 128, 0.1f},
    {77, "簇电压越下下限保护值", 3, 5396, 514, 128, 0.1f},
    {78, "簇电池绝缘电阻保护值", 3, 5397, 514, 128, 1},
    {79, "簇电池漏电流保护值", 3, 5398, 514, 128, 0.1f},
    {80, "告警延时", 3, 5399, 514, 128, 1},
    {81, "保护延时", 3, 5400, 514, 128, 1},
    {82, "簇电池容量", 3, 5401, 514, 128, 0.1f},
    {83, "簇电池校正容量", 3, 5402, 514, 128, 0.1f},
    {84, "簇电池剩余电量", 3, 5403, 514, 128, 0.1f},
    {85, "充放电额定电流", 3, 5404, 514, 128, 0.1f},
    {86, "电流传感器量程", 3, 5405, 514, 128, 1},
    {87, "漏电流传感器量程", 3, 5406, 514, 128, 1},
    {88, "电压传感器量程", 3, 5407, 514, 128, 1},
    {89, "电池均衡控制模式", 3, 5408, 514, 128, 1},
    {90, "均衡启动电压阈值", 3, 5409, 514, 128, 0.0001f},
    {91, "均衡启动电压差值", 3, 5410, 514, 128, 0.0001f},
    {92, "簇电池单元数量", 3, 5411, 514, 128, 1},
    {93, "BMU单体电池个数", 3, 5412, 514, 128, 1},
    {94, "BMU模组温度个数", 3, 5413, 514, 128, 1},
    {95, "BMU极柱温度个数", 3, 5414, 514, 128, 1},
    {96, "报警屏蔽位", 3, 5415, 514, 128, 1},
    {97, "故障屏蔽位", 3, 5416, 514, 128, 1},
    {98, "CMU功能使能位", 3, 5417, 514, 128, 1},
};
#define MAX_CFG 99
mb_cmu::mb_cmu() {
    cmu = nullptr;
    cmu_status = 0;
    stop = false;
    mb_ip = "192.168.1.120";
    mb_port = 502;
    pMq = MessageQueue::getInstance();
    pMq->registMsgQueue(0);
    tab_data.reserve(1000);
    config = {0, 0, 0, 0};
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
    int val = 0;
    float fval = 0;
    int index = -1;
    for (int i = 0; i < MAX_CFG; i++) {
        node_reg_tmp.default_val = 0;
        if (tab_config[i].reg_type > NONE_REG) {
            node_reg_tmp.reg_type = tab_config[i].reg_type;
            node_reg_tmp.reg_addr = tab_config[i].reg_addr;
            node_reg_tmp.data_type = tab_config[i].data_type;
            node_reg_tmp.index = tab_config[i].index;
            node_reg_tmp.factor = tab_config[i].factor;
            if ((index = JudgeReg(node_reg_tmp)) != -1) {
                //
                InsertReg(node_reg_tmp, index);
            } else {
                NewReg(node_reg_tmp);
            }
        }
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
    unsigned int reg_num = 0;
    uint16_t* p = this->tab_reg;
    int status = 0;
    MB_CMD cmu_config[] = {
        {0x03, 1, TAB_SYS_LEN, 0},  //时钟,状态
        {0x03, 1024, TAB_ENG_LEN, 0},
        {0x04, 5376, TAB_CMU_LEN, 0},
        {0x00, 0x00, 0, 0},
    };
    MB_CMD* pCmd = cmu_config;
    unsigned int offset = 0;
    for (; pCmd->type != 0; pCmd++) {
        offset += pCmd->reg_len;
        status += ReadData(pCmd->type, pCmd->start_addr, pCmd->reg_len, p + offset);
    }
    //
    reg_num = config.bmu_num * config.vol_num;
    status += ReadData(0x04, 0x01, reg_num, p + offset);
    offset += reg_num;
    reg_num = config.bmu_num * config.temp_num;
    status += ReadData(0x04, 0x1000, reg_num, p + offset);
    offset += reg_num;
    reg_num = config.bmu_num * config.status_num;
    status += ReadData(0x03, 0x100, reg_num, p + offset);
    offset += reg_num;
    //版本号
    reg_num = config.bmu_num * 2 + 2;
    status += ReadData(0x03, 0x500, reg_num, p + offset);
    offset += reg_num;

    if (reg_num != status) qDebug() << status << " should be " << reg_num;
    if (status <= 0) {
        cmu_status &= ~(0x01U << CMU_ONLINE);
        if (err_counter++ >= 5) {
            qDebug() << "reconnect..." << time(NULL);
            err_counter = 0;
            modbus_close(this->cmu);
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
    SM_INIT,     //
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
    STATE_MACHINE state = SM_NONE;
    while (1) {
        if (this->stop) break;
        while (pMq->readMsg(0, MsgCmd)) {
            qDebug() << "recv " << MsgCmd.msg_type << "," << MsgCmd.data;
            DealCMD(MsgCmd);
        }
        //状态机
        switch (state) {
            case SM_READ:
                if (ReadALL()) {
                    state = SM_INIT;
                }
                break;
            case SM_CONNECT: {
                if (cmu != nullptr) this->Close();
                cmu = modbus_new_tcp(this->mb_ip.c_str(), this->mb_port);
                modbus_set_slave(cmu, 1);
                modbus_set_response_timeout(cmu, 3, 0);
                if (cmu) rc = modbus_connect(this->cmu);
                if (rc == 0) state = SM_INIT;
                memset(tab_reg, 0, sizeof(tab_reg));
                // tab_config.clear();
                break;
            }
            case SM_INIT: {
                ST_SysPara sys_para;
                rc = ReadData(0x03, 5376, TAB_CFG_LEN, sys_para.array);
                if (rc == TAB_CFG_LEN) {
                    state = SM_READ;
                    config.bmu_num = sys_para.Name.u16ClusterBmuNum;
                    config.vol_num = sys_para.Name.u16BmuCellNum;
                    config.temp_num = sys_para.Name.u16BmuPackTNum + sys_para.Name.u16BmuPoleTNum;
                    config.status_num = 4;
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
            mb_ip = Msg.data.toStdString();
            qDebug() << "ip config:" << mb_ip.c_str();
        } break;
        case CONFIG_PORT: {
            mb_port = Msg.data.toInt();
            if (mb_port < 0 || mb_port > 65535) {
                mb_port = 502;
            }
            qDebug() << "port config:" << mb_port;
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
            if (Msg.data.size() == 2 * sizeof(int)) {
                int* p = (int*)Msg.data.data();
                int addr = p[0];
                int value = p[1];
                ret = modbus_write_bit(cmu, addr, value);
                if (ret != 0) qDebug() << "wr do failed" << ret;
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
                if (ret != 0) qDebug() << "wr ao failed" << ret;
            } else {
                int addr = p[0];
                uint16_t* pv = (uint16_t*)&p[1];
                ret = modbus_write_registers(cmu, addr, (nb - 1) / 2, pv);
                if (ret != 0) qDebug() << "wr aos failed" << ret;
            }
            break;
        }
        case CTRL_UPGRADE: {
            uint16_t type = Msg.data.toUShort();
            sec_ctrl(ADDR_UPGRADE, type);
        } break;
        case CTRL_ADJ: {
            uint16_t type = Msg.data.toUShort();
            sec_ctrl(ADDR_ADJ, type);
        } break;
        default:
            break;
    }
}
int mb_cmu::sec_ctrl(uint16_t addr, uint16_t type) {
    int ret = -1;
    sec_cmd[8] = type;
    ret = modbus_write_registers(cmu, addr, 9, sec_cmd);
    return ret;
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
    for (vector<DataReg>::iterator iter = reg_list_.begin(); iter != reg_list_.end(); iter++) {}
        //DEBUG_PRINT(DEBUG_ON,"start=%d,reg_num=%d-%d,io_len=%d",iter->reg_start,iter->reg_num,iter->data_num,iter->data_io.size());
//        for (int i = 0; i < 1; i++) {
//            res = modbus_re iter->reg_type, iter->reg_start, iter->reg_num, dev_id);
//            res = RecvPackCheck(dev_id, RWDeal());
//            if (res == DEF_FUNC_RESULT_OK) {
//                for (vector<DatabaseIO>::iterator data_iter = iter->data_io.begin(); data_iter != iter->data_io.end();
//                     data_iter++) {
//                    if(m_trantmpdev.node.size()>data_iter->index)
//                    {
//                        m_trantmpdev.node.at(data_iter->index).isUpdate = true;
//                        m_trantmpdev.node.at(data_iter->index).UpdateCnt++;
//                        m_trantmpdev.node.at(data_iter->index).rawdata.data.ui32 =
//                            (GetData(data_iter->offset, data_iter->data_type)).ui32;
//                        ParseSysData(m_trantmpdev.node.at(data_iter->index), data_iter->factor);
//                    }
//                    else
//                    {
//                        DEBUG_PRINT(_DEBUG, " set value error size:%d index :%d  ",m_trantmpdev.node.size(),data_iter->index);
//                    }

//                }
//                break;
//            }
//        }
//        if (res != DEF_FUNC_RESULT_OK) {
//            DEBUG_PRINT(_DEBUG, " read fail  ");
//            //return res;
//        }
//    }
//    return DEF_FUNC_RESULT_OK;
}
