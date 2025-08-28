#ifndef MB_CMU_H
#define MB_CMU_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QThread>
#include <iostream>
#include "MessageQueue.h"
#include "db_manager.h"
#include "modbus-tcp.h"
#include "modbus-version.h"
#include "node_conf.h"
#include <QObject>

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
    uint16_t reg_len;     // 寄存器长度
    uint16_t tab_offset;  // 转存表偏移
} MB_CMD;

typedef enum {
    NONE = 0,
    THREAD_EXIT,  // 线程退出
    CONFIG_INIT,  //
    CONFIG_IP,
    CONFIG_PORT,
    CTRL_DO,
    CTRL_AO,
    CTRL_SEC_AO,  // 带密钥命令
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
    CTRL_DUMPERRLOG,
    CTRL_SET_PRO,  // 设置协议版本
    CTRL_SET_ERRLOG_ULIMIT,  // 设置故障日志单体故障阈值
    CTRL_SET_ERRLOG_TLIMIT,  // 设置故障日志温度故障阈值
    CTRL_SET_ERRLOG_STDVAL,  // 设置故障日志标准差阈值
    CTRL_SET_ERRLOG_METHOD,  // 设置故障日志记录方式
    CTRL_SET_EXPRO,  // 设置扩展协议版本
} MSG_TYPE;
#define CMU_ONLINE    0
#define CMU_OUTOFDATE 31

#define TAB_SYS_LEN    12  // 系统数据:时钟,状态
#define TAB_ENG_LEN    42  // 能量数据:SOC,电量
#define TAB_CFG_LEN    11  // 配置数据:参数
#define TAB_CMU_LEN    25  // 统计数据:计算极值
#define TAB_BMU_OFFSET TAB_SYS_LEN + TAB_ENG_LEN + TAB_CMU_LEN
// 升级命令
#define ADDR_UPGRADE 0xFFD0
#define MB_UpdateCMU 0x5a78  // 23160 下载升级CMU应用程序
#define MB_UpdateBMU 0x5a33  // 23091 下载升级所有BMU应用程序
#define MB_UpdateBTB 0x5a66  // 23142 下载升级BMU BOOT程序
#define MB_UpdateBTC 0x5a6a  // 23146 下载升级CMU BOOT程序
#define MB_UpdateBFW 0xa566  // 42342 下载BMU信息文件
#define MB_UpdateCFW 0x7567  // 30055 下载CMU信息文件
#define MB_UpdBmuNDL 0xa533  // 42291 直接升级BMU应用程序
#define MB_UpdRins   0xa5b6  // 42422 下载升级绝缘板程序
// 校准命令
#define ADDR_ADJ     0xFFC0
#define MB_Adj_IZero 0x11  // 电流采样零刻度校准
#define MB_Adj_VZero 0x22  // 电压采样零刻度校准
#define MB_Adj_LZero 0x33  // 漏电流零刻度校准
#define MB_Adj_TZero 0x44  // 温度校准
#define MB_Adj_RZero 0x55  // 绝缘电阻校准

#define MB_Adj_IFull 0xaa11  // 电流采样满刻度校准
#define MB_Adj_VFull 0xaa22  // 电压采样满刻度校准
#define MB_Adj_LFull 0xaa33  // 漏电流满刻度校准
#define MB_Adj_TFull 0xaa44
#define MB_Adj_RFull 0xaa55

#define MB_Adj_IBase 0xbb11  // 电流采样基点校准
#define MB_Adj_VBase 0xbb22  // 电压采样基点校准
#define MB_Adj_LBase 0xbb33  // 漏电流基点校准
#define MB_Adj_TBase 0xbb44
#define MB_Adj_RBase 0xbb55
// 绝缘校准
#define ADDR_RINS_ADJ 0xF000
#define MB_RU_ADJ     0xCC11
#define MB_RP_ADJ     0xCC22
#define MB_RN_ADJ     0xCC33
// 其他命令
#define ADDR_TIME_ADJ 0xFFE0
#define ADDR_WR_LOCK  0xFFF0
#define MB_UNLOCK     0x67A5

#define ADDR_RESET_FACTORY 0xFFF1
#define MB_FACTORY         0x1D32
#define MB_BMU_LOCK        0xaa55
#define MB_BMU_UNLOCK      0x55aa

#define ADDR_CLEAR_ENG 0xFFF2
#define MB_CLEAR_ENG   0x1EC6
#define MB_CLEAR_ENG2  0xA5A5
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

#define ADDR_CLEAR_SYSLOCK 0xFFF9
#define MB_CLR_SYSLOCK 0xA5A5

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
#define ADDR_CTRL_HR  0xFFF7
#define ADDR_CTRL_COMM_SELF_DETECTE 0xFFFA
#define ADDR_CTRL_FINDADDR 0xFFDC
//
typedef struct {
    uint64_t soe_time;   // 事件时间
    uint16_t soe_type;   // 事件类型
    uint16_t soe_id;     // 事件ID
    uint16_t soe_val;    // 当前值
    uint16_t soe_limit;  // 限值
    uint16_t soe_stat;   // 系统状态
} CMU_SOE;
// BMU 数据
#define MAX_BMU 64
#define MAX_U   64
#define MAX_T   34
typedef struct {
    uint16_t Ucell[MAX_U];        // 单体电压
    uint16_t MaxUcellId;          // 最大单体电压ID
    uint16_t MinUcellId;          // 最小单体电压ID
    int16_t Tcell[MAX_T];         // 温度
    int16_t ModT1;                // 模块温度1
    int16_t ModT2;                // 模块温度2
    uint16_t MaxTcellId;          // 最大单体温度ID
    uint16_t MinTcellId;          // 最大单体温度ID
    uint16_t Ubreak;              // 电压断线
    uint64_t U64break;            // 液冷电压断线
    uint16_t Tbreak;              // 温度断线
    uint64_t T64break;            // 液冷温度断线
    uint16_t RunStat;             // 运行状态
    uint16_t ErrStat;             // 故障状态
    uint32_t Version;             // 版本号
    uint16_t HVersion;            // 硬件版本号
    uint32_t BMUBootVersion;      // BMUboot版本号
    uint16_t BMUSN;               // BMU生成流水号
    uint16_t BalStat;             // 均衡状态
    uint64_t U64BalStat;          // 液冷均衡状态

    uint16_t BalErr;              // 通道故障(闭锁)状态
    uint64_t U64BalErr;           // 液冷通道故障(闭锁)状态
    uint16_t BalU24;              // 均衡24V电压
    int16_t BalIdc[MAX_U];        // 均衡DC电流
    uint16_t BalI48;              // 均衡48V电流
    uint16_t BalMode;             // 均衡模式+电流
    int16_t  BalCur;              // 液冷均衡母线电流
    uint16_t CanErr;              // 通信错误计数
    uint16_t BalChgAh[MAX_U];     // 充电均衡Ah
    uint16_t BalDischgAh[MAX_U];  // 放电均衡Ah
    uint8_t FanSpeed;             // 风扇转速
} BMU_DATA_T;
typedef struct {
    uint16_t MaxUbmuId;  // 最大单体电压BMU ID
    uint16_t MinUbmuId;  // 最小单体电压BMU ID
    uint16_t MaxTbmuId;  // 最大单体温度BMU ID
    uint16_t MinTbmuId;  // 最小单体温度BMU ID

} BMS_DATA_T;
typedef struct {
    int type;  // 1:soe v1,2:soe v2
    uint16_t soe_count;
    uint16_t new_soe_count;
    QList<CMU_SOE> list_soe;
} ST_SOE;
/* 系统配置参数数据结构-------------------------------------------------------*/
#pragma pack(1)  // 此结构体不可对齐
typedef union {
    uint16_t array[11];
    struct {
        /* 以下为系统配置参数,由开发/维护人员修改---------------------------------*/
        uint16_t u16ClusterBmuNum;  // 36BMU数量1个,默认18个,范围:1-60个
        uint16_t u16BmuCellNum;     // 每个BMU单体电池数量
        uint16_t u16BmuPackTNum;    // 每个BMU模组温度个数
        uint16_t u16BmuPoleTNum;    // 每个BMU极柱温度个数
        uint16_t u16AlarmMask;      // 40报警屏蔽,默认0,0:不使用1:使用
        uint16_t u16FaultMask;      // 故障屏蔽,默认0,0:不使用1:使用
        uint16_t uFunCtrReg;        // 使能(电流/电压/漏电/绝缘/双CAN等)
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
    RD_GROUP1 = 1,
    RD_GROUP2 = 2,
} READ_GROUP;//将read all分组，不然太慢了，别的进不来

typedef enum {
    CMU_V0 = 0,//被动均衡
    CMU_V1 = 1,//主动均衡风冷MOS矩阵
    CMU_V2 = 2,//主动均衡风冷并充
    CMU_V3 = 3,//主动均衡液冷MOS矩阵
} BMS_PROTOCOL;//协议大版本--基线点表

//当前存在的所有复合版本
#define CMU_P_V0_0_00           0000U  //无法归类的版本
#define CMU_P_V0_0_01           0001U  //CMU1.0
#define CMU_P_V0_0_02           0002U  //CMU2.0
#define CMU_P_V0_0_03           0003U  //CMU3.0
#define CMU_P_V0_0_04           0004U  //CMU3.1

#define CMU_A_FAN_MOS_V1_0_00   1000U  //CMU4.1
#define CMU_A_FAN_MOS_V1_0_01   1001U  //CMU4.0
#define CMU_A_FAN_MOS_V1_0_02   1002U  //CMU4.8
#define CMU_A_FAN_MOS_V1_0_03   1003U  //CMU4.9
#define CMU_A_FAN_MOS_V1_0_04   1004U  //CMU风冷最新的基线，旧板BMStool里没有对应的版本
#define CMU_A_FAN_MOS_V1_3_00   1300U  //CMU5.0 风冷三级告警

#define CMU_A_FAN_PAL_V2_0_00   2000U  //并充

#define CMU_A_LIQ_MOS_V3_0_01   3001U  //CMU4.10
#define CMU_A_LIQ_MOS_V3_0_02   3002U  //CMU液冷最新的基线，旧板BMStool里没有对应的版本
#define CMU_A_LIQ_MOS_V3_3_00   3300U  //CMU5.1

#define CMU_ILIGAL_EXVER        000U//999U

typedef enum {
    ERRLOG_ONCE = 0,  // 一次
    ERRLOG_ALWAYS,    // 总是
    ERRLOG_NTIMES     // N次
} ERRLOG_METHOD;
typedef struct {
    uint16_t value;
    uint16_t cnt;
} ERRLOG_Data_t;


#define WR_LOCK_BIT 5
//extern QHash<QString, uint> g_proto_map;
typedef std::function<void(TMsgData &Msg)> fp_msg;
class mb_cmu : public QObject {
    Q_OBJECT
   protected:
    void DealCMD(TMsgData &Msg);

   public:
    mb_cmu(BMS_PROTOCOL ver);
    ~mb_cmu();
    virtual int Init();  // 初始化
    int ReadALL();       //
    int ReadCmuData();   //ReadALL cmu部分
    int ReadBmuData();   //ReadALL bmu部分
    int ReadCapData();   // 读容量数据
    int Close();         // 释放资源
   public:

    QHash<QString, qreal> mapData;
    QHash<QString, NodeReg> mapConfig;
    QList<db_manager::ST_DB_NODE> nodes_table;
    uint16_t tab_reg[5000];
    BMU_DATA_T bmu_data[MAX_BMU];
    BMS_DATA_T bms_data;
    ST_SysPara sys_para;
    ST_SOE cmu_soe;
    CMU_CONF config;
    uint32_t drv_status;
    uint32_t cmu_ver = 0;
    bool isWrLocked = 0;
    int max_offset;
    //    MessageQueue *pMq;
    QFile *csvfile;
    QDateTime fileTime;    
    void Dump2CsvTitle();
    void Dump2Csv();

    QFile *csvfile_errLog;
    QDateTime fileTime_errLog;    
    void DumpErrLog2CsvTitle();
    void DumpErrLog2Csv();
    QMap<uint,QList<uint16_t>>ChlCellMap;
    //  ID        故障类型        故障信息<故障值，计数>
    QMap<uint,QMap<QString,QMap<uint,uint>>>OldErrLogBufMap;
    double errLogUcellLimitValue = 0;
    double errLogTempLimitValue = 0;
    double errLogStdLimitValue = 0;

    ERRLOG_METHOD logSaveMethod = ERRLOG_ONCE;
    uint16_t errLogCount = 5;


    QString GetBitStatus(uint16_t status);
    QString GetBitStatus(uint64_t status);
    QString GetBalanceValue(uint16_t status);
    QString GetBalanceValue(uint16_t status,int16_t cur);
    QString GetBalanceValue(uint64_t status,int16_t *cur);

    BMS_PROTOCOL GetProtocalVer() { return protocal_ver; }
    int GetExProtocalVer() {return ex_ver;}
    NodeReg GetNodeAddr(QString name);
    void clearExVer() {ex_ver = CMU_ILIGAL_EXVER;}

    int compound_protocol_ver() { return int(protocal_ver)*1000 + ex_ver; }//用于协议判断的复合协议版本
    bool is_cpVer_match(int ver) {return (ver == compound_protocol_ver());}
    bool is_cpVer_Higher_than(int ver) {return (ver < compound_protocol_ver());}
    bool is_exVer_3levels_alarm() {return (ex_ver >=300);}
    bool is_pVer_passive() {return (protocal_ver == CMU_V0);}   //主版本为被动均衡
    bool is_pVer_active() {return (protocal_ver != CMU_V0);}    //主版本为主动均衡
    bool is_pVer_a_fan_mos() {return (protocal_ver == CMU_V1);}//主版本是风冷mos矩阵
    bool is_pVer_a_fan_pal() {return (protocal_ver == CMU_V2);}//主版本是风冷并充
    bool is_pVer_a_liq_mos() {return (protocal_ver == CMU_V3);}//主版本是液冷mos矩阵
    bool is_cpVer_with_fan_rate()
    {
        return (is_pVer_a_fan_pal()
                ||(is_pVer_a_fan_mos()
                && (!is_cpVer_match(CMU_A_FAN_MOS_V1_0_00))
                && (!is_cpVer_match(CMU_A_FAN_MOS_V1_0_01))
                && (!is_cpVer_match(CMU_A_FAN_MOS_V1_0_02))));
    }
    bool is_cpVer_a_fan_mos_with_boot_ver()
    {
        return (is_pVer_a_fan_mos()
                && (!is_cpVer_match(CMU_A_FAN_MOS_V1_0_00))
                && (!is_cpVer_match(CMU_A_FAN_MOS_V1_0_01)));
    }

    void setCompoundProtocolVer(int ver) {
        protocal_ver = BMS_PROTOCOL(ver/1000);
        ex_ver = ver - uint(protocal_ver*1000);
        exVerChangedFlag = true;
        emit cpVerChanged(ver);
    }
    QList<int> getCPVerList() {return cproVerList;}

    public slots:
    void msg_deal(TMsgData msg);

   protected:
    modbus_t *cmu;
    QThread *m_thread;
    int m_interval;
    QElapsedTimer statusTimeInMs;//测试用
    QMutex mutex;
    int err_counter = 0;
    enum FILE_FORMAT {
        NONE = 0,
        CSV = 0x01,
        BIN = 0x02,
    };

    int rec = 1;  // 存储格式，1:csv格式，0x02:bin格式（json+zip压缩）
    STATE_MACHINE state = SM_NONE;
    READ_GROUP    currentReadGroup = RD_GROUP1;

    string mb_ip;
    int mb_port;
    // 配置表
    BMS_PROTOCOL protocal_ver;
    int ex_ver;
    bool exVerChangedFlag = false;
    QList<int> cproVerList;

    bool stop;
    bool stopDump;              // 停止保存数据
    bool stopDumpErrLog;        // 停止保存故障数据
    vector<DataReg> reg_list_;  // 读取表
    vector<NodeReg> wr_list_;   // 下发表
    int ReadAI();
    int ReadSOE();
    int JudgeReg(NodeReg &node_reg);
    void NewReg(NodeReg &node_reg);
    void InsertReg(NodeReg &node_reg, int index);
    int ReadData(uint8_t type, int start, int len, uint16_t *dest);
    int sec_ctrl(uint16_t addr, uint16_t type);
    int sec_ctrl(uint16_t addr, uint16_t type,uint16_t value);
    int write_ao(uint16_t addr, uint16_t len, uint16_t *pv);
    int write_ao(uint16_t addr, uint16_t v);
    int ParseData();

    void timerEvent(QTimerEvent *event);
   signals:
    void signal_message(const QString &msg);
    void bmuDataReady();
    void bmsDataReady(int type, QHash<QString, qreal> mapData);
    void bmsSOEReady(ST_SOE soe);
    void connectChanged(const QString &conn);

    void cpVerChanged(int cpVer);
private:
    bool IsErrCanWrite(int id, QString err_type, uint val);

public:
    //主动均衡
    const QList<QString> BmuStateStr_a = {tr("存储地址与拨码地址不一致或自动寻址中"),tr("地址锁定或使用自动地址"),tr("干接点开路"),tr("风机开机"),
                                 tr("+5V 辅源状态正常"),tr(""),tr(""),tr(""),
                                 tr("地址拨码一致或自动寻址完成"),tr("地址未锁定或使用拨码地址"),tr("干接点闭合"),tr("风机关机"),
                                 tr("+5V 辅源状态异常"),tr(""),tr(""),tr("")};

    const QList<QString> BmuBalErrStr_a = {tr("1.25V 参考电压错误"),tr("均衡前正母线电压错误"),tr("均衡电流异常"),tr("24V母线电压超范围"),
                                 tr("单体电压超范围"),tr("均衡参数错误"),tr("极性Mos故障"),tr("均衡前副边电压错误")};

    const QList<QString> BmuErrStr_a = {tr("采样芯片处于配置状态"),tr("BMU处于均衡状态"),tr("采样芯片时钟异常"),tr("温度低温异常"),
                               tr("温度过温异常"),tr("单体过压故障汇总"),tr("单体欠压故障汇总"),tr("单体断线故障汇总(B1-,B1+~B16+)"),
                               tr("采样线总正断线故障"),tr("采样线总负断线故障"),tr("采样芯片寄存器校验异常"),tr("采样芯片参考电压异常"),
                               tr("采样芯片校准电压异常"),tr("采样芯片复用采样通道异常"),tr("采样芯片故障1"),tr("采样芯片故障2")};

    //并充
    const QList<QString> BmuStateStr_m = {tr("自动寻址过程中"),tr("地址锁定"),tr("干接点开路"),tr("风机开机"),
                                 tr("均衡母线关闭"),tr(""),tr(""),tr(""),
                                 tr("自动寻址完成"),tr("地址未锁定"),tr("干接点闭合"),tr("风机关机"),
                                 tr("均衡母线打开"),tr(""),tr(""),tr("")};

    const QList<QString> BmuBalErrStr_m = {tr("均衡设置参数错误"),tr("均衡母线零点电流异常"),tr("均衡母线均衡电流异常"),tr("通道均衡电流异常"),
                                 tr("均衡母线电压异常"),tr("均衡通道DCDC模块开启失败"),tr("均衡通道DCDC模块关闭失败"),tr("均衡通道DCDC输出电压异常")};

    const QList<QString> BmuErrStr_m = {tr("采样芯片故障"),tr("采样芯片参考电压异常(VREF)"),tr("采样芯片参数配置异常"),tr("单体电压断线故障汇总(B1+~B56+)"),
                               tr("电池温度断线故障汇总"),tr("采样线总正断线故障"),tr("采样线总负断线故障"),tr("采样芯片电压采样通道异常"),
                               tr("采样芯片温度采样通道异常"),tr("PCB板温度异常"),tr("EEPROM故障"),tr("G03离线告警"),
                               tr("运行前片选I0控制异常"),tr("均衡过程中片选I0控制异常"),tr("bit14"),tr("bit15")};

    //液冷
    const QList<QString> BmuStateStr_w = {tr("自动寻址过程中"),tr("地址锁定"),tr("干接点开路"),tr("地址输入状态H"),
                                 tr("AFE_A_5V辅源有效"),tr("AFE_B_5V辅源有效"),tr("AFE_C_5V辅源有效"),tr("AFE_D_5V辅源有效"),
                                 tr("自动寻址完成"),tr("地址未锁定"),tr("干接点闭合"),tr("地址输入状态L"),
                                 tr("AFE_A_5V辅源无效"),tr("AFE_B_5V辅源无效"),tr("AFE_C_5V辅源无效"),tr("AFE_D_5V辅源无效")};

    const QList<QString> BmuBalErrStr_w = {tr("24V母线电流过流"),tr("均衡前1.25V参考电压故障"),tr("均衡前总线正对地电压故障"),tr("均衡中电流与目标电流差值过大"),
                                 tr("均衡前24V母线电压故障"),tr("均衡前单体电压越界"),tr("均衡中VP,VB,IP异常"),tr("均衡前副边电压检测异常")};

    const QList<QString> BmuErrStr_w = {tr("采样芯片故障"),tr("采样芯片参考电压异常(VREF)"),tr("采样芯片参数配置异常"),tr("单体电压断线故障汇总(B1+~B56+)"),
                               tr("电池温度断线故障汇总"),tr("采样线总正断线故障"),tr("采样线总负断线故障"),tr("采样芯片电压采样通道异常"),
                               tr("采样芯片温度采样通道异常"),tr("PCB板温度异常"),tr("EEPROM故障"),tr("G03离线告警"),
                               tr(""),tr(""),tr(""),tr("")};
};
#endif  // MB_CMU_H
