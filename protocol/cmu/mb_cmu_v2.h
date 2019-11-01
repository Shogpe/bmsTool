#ifndef MB_CMU_V2_H
#define MB_CMU_V2_H

#include <QThread>
#include <iostream>

#include "MessageQueue.h"
#include "modbus-tcp.h"
#include "modbus-version.h"
#include "modbus.h"
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
#define GET_RAWDATALEN(x)     ((x & 0x0f00) >> 8)
#define GET_RAWDATATYPE_ID(x) (x & 0xf)
//自动采集解析结构
typedef struct structDatabaseIO {
    uint16_t data_type;  //数据器类型
    uint8_t offset;      //在返回串中的位/字序号
    double factor;       //变比
    uint16_t index;      //实时数据地址
} DatabaseIO;
//采集结构体
#define IO_MAX   200
#define NONE_REG 0x00
#define DO_REG   0x01
#define DI_REG   0x02
#define AO_REG   0x03
#define AI_REG   0x04
typedef struct structReg {
    int dev_id;                  //设备地址
    unsigned char reg_type;      //寄存器类型，功能码
    int reg_start;               //起始地址
    int reg_num;                 //连续个数
    int data_num;                //数据个数
    vector<DatabaseIO> data_io;  //实时数据库,连续个数
} DataReg;
//写入结构体
typedef struct structTable {
    uint index;              //数据库序号
    unsigned char reg_type;  //寄存器类型，功能码
    int reg_addr;            //寄存器地址
    int data_type;           //数据类型
    double default_val;      //初值
    float factor;            //变比
} NodeReg;
typedef struct {
    int index;           // 数据索引
    string name;         // 控件名
    uint16_t reg_type;   // 数据类型
    uint16_t reg_addr;   // 数据类型
    uint32_t data_type;  // 数据类型
    uint32_t val_type;   // 数据类型
    double factor;       //变比
} MB_NODE;
// 32位系统数据类型定义
typedef union {
    unsigned char b[4];
    int32_t i32;
    float f32;
    uint32_t ui32;
    int16_t i16;
    uint16_t ui16;
    int16_t i16_array[2];
    uint16_t ui16_array[2];
} DT_RAW32;

// 64位数据结构定义
typedef union {
    unsigned char b[8];
    uint16_t ui16_array[4];
    int16_t i16_array[4];
    int32_t i32_array[2];
    uint32_t ui32_array[2];
    DT_RAW32 st_32type[2];
    int32_t i32;
    float f32;
    double f64;
    uint32_t ui32;
    int16_t i16;
    uint16_t ui16;
    uint64_t ui64;
    int64_t i64;
    void *p;
} DT_RAW64;
typedef struct {
    DT_RAW64 val;           //数据值
    time_t t;               //数据时间
    unsigned char valtype;  //数据类型 AI DI ACC AIwithT DIwithT ACCwithT
} ST_SYS_DATA;
//原始数据类型定义
typedef struct {
    DT_RAW64 data;  //点数据
    uint16_t type;  //数据类型
} ST_POINT_DATA;
//节点数据结构
typedef struct {
    uint16_t isUpdate;      //是否被跟新  1更新 其他没有跟新
    uint16_t UpdateCnt;     //更新计数
    ST_POINT_DATA rawdata;  //原始数据
    ST_SYS_DATA sysData;    //转化为系统格式数据
} ST_NODE_DATA;

typedef enum {
    NONE = 0,
    THREAD_EXIT,  //线程退出
    CONFIG_INIT,  //
    CONFIG_IP,
    CONFIG_PORT,
    CTRL_DO,
    CTRL_AO,
    CTRL_DOWN_BMS,
    CTRL_DOWN_BMS_BTL,  // BMS BOOTLOADER
    CTRL_DOWN_BMU,
    CTRL_DOWN_BMU_BTL,  // BMU BTL
    CTRL_UPGRADE_BMU,   //升级BMU,不下载
    CTRL_ADJ_U_FULL,
    CTRL_ADJ_U_ZERO,
    CTRL_ADJ_I_FULL,
    CTRL_ADJ_I_ZERO,
    CTRL_ADJ_ILEAK_FULL,
    CTRL_ADJ_ILEAK_ZERO,
    CTRL_ADJ_RINS_FULL,
    CTRL_ADJ_RINS_ZERO,
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
//校准命令
#define ADDR_ADJ     0xFFC0
#define MB_Adj_IZero 0x11    //电流采样零刻度校准
#define MB_Adj_VZero 0x22    //电压采样零刻度校准
#define MB_Adj_LZero 0x33    //漏电流零刻度校准
#define MB_Adj_IFull 0xaa11  //电流采样满刻度校准
#define MB_Adj_VFull 0xaa22  //电压采样满刻度校准
#define MB_Adj_LFull 0xaa33  //漏电流满刻度校准
#define MB_Adj_TZero 0x44    //温度校准
#define MB_Adj_TFull 0xaa44
#define MB_Adj_RZero 0x55  //绝缘电阻校准
#define MB_Adj_RFull 0xaa55
//其他命令
#define ADDR_TIME_ADJ      0xFFE0
#define ADDR_WR_LOCK       0xFFF0
#define MB_UNLOCK          0x67A5
#define ADDR_RESET_FACTORY 0xFFF1
#define MB_FACTORY         0x1D32
#define MB_BMU_LOCK        0x55aa
#define MB_BMU_UNLOCK      0xaa55
#define ADDR_CLEAR_ENG     0xFFF2
#define MB_CLEAR_ENG       0x1EC6
#define ADDR_REBOOT        0xFFF3
#define MB_REBOOT          0x1D32
#define ADDR_CLEAR_SOE     0xFFF8
#define MB_CLR_SOE         0xAA55
#define MB_CLR_ALL_SOE     0xBB66
//
typedef struct {
    uint64_t soe_time;   // 事件时间
    uint16_t soe_type;   // 事件类型
    uint16_t soe_id;     //事件ID
    uint16_t soe_val;    //当前值
    uint16_t soe_limit;  // 限值
    uint16_t soe_stat;   // 系统状态
} CMU_SOE;
typedef struct {
    uint16_t soe_count;
    uint16_t new_soe_count;
    CMU_SOE list_soe[500];
} ST_SOE;
/* 系统配置参数数据结构-------------------------------------------------------*/
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
        uint32_t u32LocalIP;  // 43本地IP低位,192.168 0xa8c0  2143格式
        uint32_t u32TftpServIP;  // TFTP服务器地址低位192.168 0xA8C0 2143格式
    } Name;
} ST_SysPara;
typedef enum {
    SM_NONE = 0,
    SM_CONNECT,  //
    SM_READ,     //
    SM_CTRL,     //
    SM_INIT,     //
} STATE_MACHINE;
class mb_cmu_v2 : public QThread {
    Q_OBJECT
   protected:
    void run();
    void DealCMD(TMsgData &Msg);

   public:
    mb_cmu_v2();
    ~mb_cmu_v2();
    int Init();     //初始化
    int ReadALL();  //
    int Close();    //释放资源
   public:
    uint16_t tab_reg[1000];
    uint16_t tab_AI[1000];
    ST_SysPara sys_para;
    ST_SOE cmu_soe;
    CMU_CONF config;
    vector<ST_NODE_DATA> tab_data;
    uint32_t cmu_ver = 0;
    uint32_t cmu_status;
    int max_offset;
    MessageQueue *pMq;
    map<string, NodeReg> name_map;

   private:
    modbus_t *cmu;
    int err_counter = 0;
    STATE_MACHINE state = SM_NONE;
    string mb_ip;
    int mb_port;
    bool stop;
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

#endif  // MB_CMU_V2_H
