#ifndef MB_CMU_H
#define MB_CMU_H

#include <QDateTime>
#include <QFile>
#include <QThread>
#include <iostream>
#include "MessageQueue.h"
#include "modbus-tcp.h"
#include "modbus-version.h"
#include "node_conf.h"
using namespace std;

typedef struct {
    uint16_t bmu_num;     // bmu个数
    uint16_t vol_num;     // bmu电压个数
    uint16_t T_num;       // bmu温度个数
    uint16_t Tp_num;      // bmu温度个数
    uint16_t status_num;  // bmu状态个数
} CMU_CONF;

typedef struct {
    uint8_t type;         // 寄存器类型
    uint16_t start_addr;  // 寄存器起始地址
    uint16_t reg_len;     //寄存器长度
    uint16_t tab_offset;  // 转存表偏移
} MB_CMD;

typedef enum {
    NONE = 0,
    THREAD_EXIT,  //线程退出
    CONFIG_INIT,  //
    CONFIG_IP,
    CONFIG_PORT,
    CTRL_DO,
    CTRL_AO,
    CTRL_SEC_AO,  //带密钥命令
    CTRL_CMD_CLR_ENG,
    CTRL_CMD_CLR_SOE,
    CTRL_CMD_CLR_ALL_SOE,
    CTRL_CMD_UNLOCK,
    CTRL_CMD_BMU_LOCK,
    CTRL_CMD_BMU_UNLOCK,
    CTRL_CMD_RESET,
    CTRL_CMD_REBOOT,
    CERT_CMD_TIME_ADJ,
    CERT_CMD_READ_SOE,
    CTRL_AO_ADDR,
    CTRL_DUMP,
    CTRL_SET_PRO,  //设置协议版本
} MSG_TYPE;
#define CMU_ONLINE    0
#define CMU_OUTOFDATE 31

#define TAB_SYS_LEN    12  //系统数据:时钟,状态
#define TAB_ENG_LEN    42  //能量数据:SOC,电量
#define TAB_CFG_LEN    11  //配置数据:参数
#define TAB_CMU_LEN    25  //统计数据:计算极值
#define TAB_BMU_OFFSET TAB_SYS_LEN + TAB_ENG_LEN + TAB_CMU_LEN
//升级命令
#define ADDR_UPGRADE 0xFFD0
#define MB_UpdateCMU 0x5a78  // 23160 下载升级CMU应用程序
#define MB_UpdateBMU 0x5a33  // 23091 下载升级所有BMU应用程序
#define MB_UpdateBTB 0x5a66  // 23142 下载升级BMU BOOT程序
#define MB_UpdateBTC 0x5a6a  // 23146 下载升级CMU BOOT程序
#define MB_UpdateBFW 0xa566  // 42342 下载BMU信息文件
#define MB_UpdateCFW 0x7567  // 30055 下载CMU信息文件
#define MB_UpdBmuNDL 0xa533  // 42291 直接升级BMU应用程序
#define MB_UpdRins   0xa5b6  // 42422 下载升级绝缘板程序
//校准命令
#define ADDR_ADJ     0xFFC0
#define MB_Adj_IZero 0x11  //电流采样零刻度校准
#define MB_Adj_VZero 0x22  //电压采样零刻度校准
#define MB_Adj_LZero 0x33  //漏电流零刻度校准
#define MB_Adj_TZero 0x44  //温度校准
#define MB_Adj_RZero 0x55  //绝缘电阻校准

#define MB_Adj_IFull 0xaa11  //电流采样满刻度校准
#define MB_Adj_VFull 0xaa22  //电压采样满刻度校准
#define MB_Adj_LFull 0xaa33  //漏电流满刻度校准
#define MB_Adj_TFull 0xaa44
#define MB_Adj_RFull 0xaa55

#define MB_Adj_IBase 0xbb11  //电流采样基点校准
#define MB_Adj_VBase 0xbb22  //电压采样基点校准
#define MB_Adj_LBase 0xbb33  //漏电流基点校准
#define MB_Adj_TBase 0xbb44
#define MB_Adj_RBase 0xbb55
//绝缘校准
#define ADDR_RINS_ADJ 0xF000
#define MB_RU_ADJ     0xCC11
#define MB_RP_ADJ     0xCC22
#define MB_RN_ADJ     0xCC33
//其他命令
#define ADDR_TIME_ADJ 0xFFE0
#define ADDR_WR_LOCK  0xFFF0
#define MB_UNLOCK     0x67A5

#define ADDR_RESET_FACTORY 0xFFF1
#define MB_FACTORY         0x1D32
#define MB_BMU_LOCK        0xaa55
#define MB_BMU_UNLOCK      0x55aa

#define ADDR_CLEAR_ENG 0xFFF2
#define MB_CLEAR_ENG   0x1EC6
#define MB_UPLOAD_Trig 0x1D32

#define ADDR_REBOOT   0xFFF3
#define MB_REBOOT     0x1D32
#define MB_REBOOT_BMU 0xAA55

#define ADDR_IO_EN   0xFFF4
#define MB_IO_UNLOCK 0xA5B6
#define MB_IO_LOCK   0x0

#define ADDR_CLEAR_SOE 0xFFF8
#define MB_CLR_SOE     0xAA55
#define MB_CLR_ALL_SOE 0xBB66

#define ADDR_CTRL_AUTO 0x2000
#define MB_CTRL_ON     0xAA55
#define MB_CTRL_OFF    0x55AA

#define ADDR_CTRL_KMP 0xFF00
#define ADDR_CTRL_KMN 0xFF01
#define ADDR_CTRL_KMR 0xFF02
#define ADDR_CTRL_QF  0xFF03
#define ADDR_CTRL_FAN 0xFF04
#define ADDR_CTRL_AC  0xFF05
#define ADDR_CTRL_RES 0xFF06
//
typedef struct {
    uint64_t soe_time;   // 事件时间
    uint16_t soe_type;   // 事件类型
    uint16_t soe_id;     //事件ID
    uint16_t soe_val;    //当前值
    uint16_t soe_limit;  // 限值
    uint16_t soe_stat;   // 系统状态
} CMU_SOE;
// BMU 数据
#define MAX_U 32
#define MAX_T 8
typedef struct {
    uint16_t Ucell[MAX_U];        // 单体电压
    int16_t Tcell[MAX_T];         // 温度
    uint16_t Ubreak;              // 电压断线
    uint16_t Tbreak;              // 温度断线
    uint16_t RunStat;             // 运行状态
    uint16_t ErrStat;             // 故障状态
    uint32_t Version;             // 版本号
    uint16_t BalStat;             // 均衡状态
    uint16_t BalErr;              // 通道故障(闭锁)状态
    uint16_t BalU24;              // 均衡24V电压
    int16_t BalIdc;               // 均衡DC电流
    uint16_t BalMode;             // 均衡模式+电流
    uint16_t CanErr;              // 通信错误计数
    uint16_t BalChgAh[MAX_U];     // 充电均衡Ah
    uint16_t BalDischgAh[MAX_U];  // 放电均衡Ah
} BMU_DATA_T;
typedef struct {
    uint16_t soe_count;
    uint16_t new_soe_count;
    CMU_SOE list_soe[500];
} ST_SOE;
/* 系统配置参数数据结构-------------------------------------------------------*/
#pragma pack(1)  // 此结构体不可对齐
typedef union {
    uint16_t array[11];
    struct {
        /* 以下为系统配置参数,由开发/维护人员修改---------------------------------*/
        uint16_t u16ClusterBmuNum;  // 36BMU数量1个,默认18个,范围:1-60个
        uint16_t u16BmuCellNum;     //每个BMU单体电池数量
        uint16_t u16BmuPackTNum;    //每个BMU模组温度个数
        uint16_t u16BmuPoleTNum;    //每个BMU极柱温度个数
        uint16_t u16AlarmMask;      // 40报警屏蔽,默认0,0:不使用1:使用
        uint16_t u16FaultMask;      //故障屏蔽,默认0,0:不使用1:使用
        uint16_t uFunCtrReg;        //使能(电流/电压/漏电/绝缘/双CAN等)
        uint32_t u32LocalIP;        // 43本地IP低位,192.168 0xa8c0  2143格式
        uint32_t u32TftpServIP;     // TFTP服务器地址低位192.168 0xA8C0 2143格式
    } Name;
} ST_SysPara;
#pragma pack()
typedef enum {
    SM_NONE = 0,
    SM_CONNECT,  //
    SM_READ,     //
    SM_CTRL,     //
    SM_INIT,     //
} STATE_MACHINE;
typedef enum {
    CMUV1 = 0,
    CMUV2,    //
    CMUV3,    //
    CMUV4,    //主动均衡
    CMUV4_1,  //主动均衡-对外
    CMUV4_8,  //主动均衡-绝缘
} BMS_PROTOCOL;
#define WR_LOCK_BIT 5
typedef std::function<void(TMsgData &Msg)> fp_msg;
class mb_cmu : public QThread {
    Q_OBJECT
   protected:
    virtual void run();
    void DealCMD(TMsgData &Msg);

   public:
    mb_cmu();
    mb_cmu(BMS_PROTOCOL ver);
    ~mb_cmu();
    virtual int Init();  //初始化
    int ReadALL();       //
    int ReadCapData();   //读容量数据
    int Close();         //释放资源
   public:
    uint16_t tab_reg[1000];
    BMU_DATA_T bmu_data[60];
    ST_SysPara sys_para;
    ST_SOE cmu_soe;
    CMU_CONF config;
    uint32_t drv_status;
    vector<ST_NODE_DATA> tab_data;
    uint32_t cmu_ver = 0;
    bool isWrLocked = 0;
    int max_offset;
    MessageQueue *pMq;
    map<string, NodeReg> name_map;
    QFile *csvfile;
    QDateTime fileTime;
    void Dump2CsvTitle();
    void Dump2Csv();
    QString GetBalanceStatus(uint16_t status);
    QString GetBalanceValue(uint16_t status);
    BMS_PROTOCOL GetProtocalVer() { return protocal_ver; }

   protected:
    modbus_t *cmu;
    int err_counter = 0;
    enum FILE_FORMAT {
        NONE = 0,
        CSV = 0x01,
        BIN = 0x02,
    };

    int rec = 1;  // 存储格式，1:csv格式，0x02:bin格式（json+zip压缩）
    STATE_MACHINE state = SM_NONE;
    string mb_ip;
    int mb_port;
    // 配置表
    MB_NODE *node_table;
    int node_table_size;
    BMS_PROTOCOL protocal_ver;
    bool stop;
    bool stopDump;              //停止保存数据
    vector<DataReg> reg_list_;  //读取表
    vector<NodeReg> wr_list_;   //下发表
    int ReadAI();
    int ReadSOE();
    int JudgeReg(NodeReg &node_reg);
    void NewReg(NodeReg &node_reg);
    void InsertReg(NodeReg &node_reg, int index);
    int ReadData(uint8_t type, int start, int len, uint16_t *dest);
    int sec_ctrl(uint16_t addr, uint16_t type);
    int write_ao(uint16_t addr, uint16_t len, uint16_t *pv);
    int write_ao(uint16_t addr, uint16_t v);
    int ParseData();
   signals:
    void signal_message(const QString &msg);
};

#endif  // MB_CMU_H
