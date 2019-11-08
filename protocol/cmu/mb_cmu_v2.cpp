#include <QDebug>
#include <QTimerEvent>
#include "mb_cmu.h"
#include "myhelper.h"
#include "utils.h"

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
    {10, "UdMax", 4, 5386, 513, 128, 0.0001},
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
    {23, "ClusterT1", 4, 5399, 513, 128, 0.1},
    {24, "ClusterT2", 4, 5400, 513, 128, 0.1},
    {25, "ClusterT3", 4, 5401, 513, 128, 0.1},
    {26, "ClusterT4", 4, 5402, 513, 128, 0.1},
    {27, "ClusterT5", 4, 5403, 513, 128, 0.1},
    {28, "sysTime", 3, 1, 17410, 128, 1},
    {29, "sysStatus1", 3, 3, 514, 128, 1},
    {30, "sysStatus2", 3, 4, 514, 128, 1},
    {31, "sysErrStatus", 3, 5, 514, 128, 1},
    {32, "sysErrStatus2", 3, 6, 514, 128, 1},
    {33, "sysAlmStatus", 3, 7, 514, 128, 1},
    {34, "sysAlmStatus2", 3, 8, 514, 128, 1},
    {35, "sysComm1", 3, 9, 17410, 128, 1},
    {36, "sysComm2", 3, 11, 17410, 128, 1},
    {37, "sysTBreak", 3, 13, 514, 128, 1},
    {38, "sysDOStatus", 3, 14, 514, 128, 1},
    {39, "sysDIStatus", 3, 15, 514, 128, 1},
    {40, "SOC", 3, 1024, 514, 128, 0.1},
    {41, "SOH", 3, 1025, 514, 128, 0.1},
    {42, "Pdc", 3, 1026, 513, 128, 0.1},
    {43, "ERemain", 3, 1027, 17410, 128, 0.1},
    {44, "ECharge", 3, 1029, 17410, 128, 0.1},
    {45, "EDischarge", 3, 1031, 17410, 128, 0.1},
    {46, "ECurCharge", 3, 1033, 17410, 128, 0.1},
    {47, "ECurDischarge", 3, 1035, 17410, 128, 0.1},
    {48, "当前剩余库伦", 3, 1037, 17410, 128, 0.1},
    {49, "当前输入库伦", 3, 1039, 17410, 128, 0.1},
    {50, "当前输出库伦", 3, 1041, 17410, 128, 0.1},
    {51, "累计输入库伦", 3, 1043, 17410, 128, 0.1},
    {52, "累计输出库伦", 3, 1045, 17410, 128, 0.1},
    {53, "TCharge", 3, 1047, 17410, 128, 1},
    {54, "TDischarge", 3, 1049, 17410, 128, 1},
    {55, "TCurCharge", 3, 1051, 17410, 128, 1},
    {56, "TCurDischarge", 3, 1053, 17410, 128, 1},
    {57, "CountCharge", 3, 1055, 17410, 128, 1},
    {58, "CountDischarge", 3, 1057, 17410, 128, 1},
    {59, "充电开始时间", 3, 1059, 17410, 128, 1},
    {60, "充电停止时间", 3, 1061, 17410, 128, 1},
    {61, "放电开始时间", 3, 1063, 17410, 128, 1},
    {62, "放电停止时间", 3, 1065, 17410, 128, 1},
    {63, "PreChgT", 3, 5358, 514, 128, 1},
    {64, "PreChgI", 3, 5359, 514, 128, 0.1},
    {65, "PreSensorRg", 3, 5360, 514, 128, 1},
    {66, "RateAH", 3, 5361, 514, 128, 0.1},
    {67, "IStaTH", 3, 5362, 514, 128, 0.1},
    {68, "VFulDF", 3, 5363, 514, 128, 1},
    {69, "VEmpDF", 3, 5364, 514, 128, 1},
    {70, "EvmDfT", 3, 5365, 513, 128, 0.1},
    {71, "TEvmH", 3, 5366, 513, 128, 0.1},
    {72, "TEvmL", 3, 5367, 513, 128, 0.1},
    {73, "SocL", 3, 5368, 514, 128, 0.1},
    {74, "SocLL", 3, 5369, 514, 128, 0.1},
    {75, "CPoleTH", 3, 5370, 513, 128, 0.1},
    {76, "CPoleTHH", 3, 5371, 513, 128, 0.1},
    {77, "ClusterRInsH", 3, 5372, 514, 128, 1},
    {78, "ClusterCurLeakH", 3, 5373, 514, 128, 0.1},
    {79, "CellUdH", 3, 5374, 514, 128, 0.0001},
    {80, "CellUdHH", 3, 5375, 514, 128, 0.0001},
    {81, "CellVolH", 3, 5376, 514, 128, 0.0001},
    {82, "CellVolHH", 3, 5377, 514, 128, 0.0001},
    {83, "CellVolL", 3, 5378, 514, 128, 0.0001},
    {84, "CellVolLL", 3, 5379, 514, 128, 0.0001},
    {85, "PackTH", 3, 5380, 513, 128, 0.1},
    {86, "PackTHH", 3, 5381, 513, 128, 0.1},
    {87, "PackTL", 3, 5382, 513, 128, 0.1},
    {88, "PackTLL", 3, 5383, 513, 128, 0.1},
    {89, "PackTdH", 3, 5384, 513, 128, 0.1},
    {90, "PackTdHH", 3, 5385, 513, 128, 0.1},
    {91, "PackTrH", 3, 5386, 513, 128, 0.1},
    {92, "PackTrHH", 3, 5387, 513, 128, 0.1},
    {93, "PoleTH", 3, 5388, 513, 128, 0.1},
    {94, "PoleTHH", 3, 5389, 513, 128, 0.1},
    {95, "ClusterCurH", 3, 5390, 514, 128, 1},
    {96, "ClusterCurHH", 3, 5391, 514, 128, 1},
    {97, "ClusterCurShort", 3, 5392, 514, 128, 1},
    {98, "ClusterVolH", 3, 5393, 514, 128, 0.1},
    {99, "ClusterVolHH", 3, 5394, 514, 128, 0.1},
    {100, "ClusterVolL", 3, 5395, 514, 128, 0.1},
    {101, "ClusterVolLL", 3, 5396, 514, 128, 0.1},
    {102, "ClusterRIns", 3, 5397, 514, 128, 1},
    {103, "ClusterCurLeak", 3, 5398, 514, 128, 0.1},
    {104, "ClusterTAlm", 3, 5399, 514, 128, 1},
    {105, "ClusterTErr", 3, 5400, 514, 128, 1},
    {106, "ClusterE", 3, 5401, 514, 128, 0.1},
    {107, "ClusterEAdj", 3, 5402, 514, 128, 0.1},
    {108, "ClusterEremain", 3, 5403, 514, 128, 0.1},
    {109, "ClusterIe", 3, 5404, 514, 128, 0.1},
    {110, "ClusterCurRange", 3, 5405, 514, 128, 1},
    {111, "ClusterILeakRg", 3, 5406, 514, 128, 1},
    {112, "ClusterVolRange", 3, 5407, 514, 128, 1},
    {113, "BalnceMask", 3, 5408, 514, 128, 1},
    {114, "BalnceStart", 3, 5409, 514, 128, 0.0001},
    {115, "BalnceStartDiff", 3, 5410, 514, 128, 0.1},
    {116, "ClusterBmuNum", 3, 5411, 514, 128, 1},
    {117, "BmuCellNum", 3, 5412, 514, 128, 1},
    {118, "BmuPackTNum", 3, 5413, 514, 128, 1},
    {119, "BmuPoleTNum", 3, 5414, 514, 128, 1},
    {120, "ClusterAlmMask", 3, 5415, 514, 128, 1},
    {121, "ClusterErrMask", 3, 5416, 514, 128, 1},
    {122, "FuncMask", 3, 5417, 514, 128, 1},
    {123, "IP", 3, 5418, 17410, 128, 1},
    {124, "ServerIP", 3, 5420, 17410, 128, 1},
};
#define MAX_CFG 125

int mb_cmu_v2::Init() {
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

#define TIME_OUTOFDATE 1 * 31 * 24 * 60 * 60

void mb_cmu_v2::run() {
    qDebug() << time(nullptr);
    if (time(nullptr) > (myHelper::cvt_TIME(__DATE__) + TIME_OUTOFDATE)) {
        qDebug() << "timeout exit..";
        this->stop = true;
        cmu_status |= (0x01 << CMU_OUTOFDATE);
    }
    int rc = -1;
    TMsgData MsgCmd;

    this->Init();
    qDebug() << name_map.size();
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
                rc = ReadData(0x03, 5411, sizeof(sys_para) / 2, sys_para.array);
                if (rc == sizeof(sys_para) / 2) {
                    state = SM_READ;
                    sys_para.Name.u32LocalIP = bswap_32(sys_para.Name.u32LocalIP);
                    sys_para.Name.u32TftpServIP = bswap_32(sys_para.Name.u32TftpServIP);
                    isWrLocked = (sys_para.Name.uFunCtrReg &(0x01 << WR_LOCK_BIT)) > 0?true:false;
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
        usleep(200 * 1000);
    }
    qDebug() << "cmu exit..";
}
