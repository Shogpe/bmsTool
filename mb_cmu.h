#ifndef MB_CMU_H
#define MB_CMU_H

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
    uint16_t temp_num;    // bmu温度个数
    uint16_t status_num;  // bmu状态个数
} CMU_CONF;

typedef struct {
    uint8_t type;         // 寄存器类型
    uint16_t start_addr;  // 寄存器起始地址
    uint16_t reg_len;     //寄存器长度
    uint16_t tab_offset;  // 转存表偏移
} MB_CMD;

typedef struct {
    int index;           // 数据索引
    uint32_t data_type;  // 数据类型
    float factor;        //变比
    string name;         // 控件名
} MB_NODE;

typedef enum {
    NONE = 0,
    THREAD_EXIT,   //线程退出
    CONFIG_INIT,  //
    CONFIG_IP,
    CONFIG_PORT,
} MSG_TYPE;
#define CMU_ONLINE 0
#define CMU_OUTOFDATE 31

#define TAB_SYS_LEN 12  //系统数据:时钟,状态
#define TAB_ENG_LEN 42  //能量数据:SOC,电量
#define TAB_CFG_LEN 46  //配置数据:参数
#define TAB_CMU_LEN 25  //统计数据:计算极值
/* 系统配置参数数据结构-------------------------------------------------------*/
typedef union {
    uint16_t array[46];
    struct {
        //单体电压0.0001V
        uint16_t u16CellVolAlmLimitH;  // 1单体高压报警,默认3.55V,3.00-4.20V
        uint16_t u16CellVolErrLimitH;  //单体高压故障,默认3.65V,3.30-4.20V
        uint16_t u16CellVolAlmLimitL;  //单体低压报警,默认2.8V,范围1.8-3.3V
        uint16_t u16CellVolErrLimitL;  //单体低压故障,默认2.6V,范围1.8-3.0V
        //模组高低温0.1℃
        int16_t i16PackTAlmLimitH;  // 5单体温度报警,默认45℃,范围：20-70℃
        int16_t i16PackTErrLimitH;  //单体温度故障,默认55℃,范围：55-70℃
        int16_t i16PackTAlmLimitL;  //单体温度报警,默认5℃,范围：-15-20℃
        int16_t i16PackTErrLimitL;  //单体温度故障 ,默认-5℃,范围：-15-0℃
        //模组温差
        int16_t i16PackTdAlmLimit;  // 9温差报警,默认10℃,范围：3-20℃
        int16_t i16PackTdErrLimit;  //温差故障,默认14℃,范围：10-20℃
        //模组温升0.1℃/Min
        int16_t i16PackTrAlmLimit;  // 11单体温升上限报警,默认4℃/Min,0-12℃
        int16_t i16PackTrErrLimit;  //单体温升上限故障,默认6℃/Min,8-12℃
        //极柱温度
        int16_t i16PoleTAlmLimitH;  // 13极柱温度上限告警值,默认50℃,25-75℃
        int16_t i16PoleTErrLimitH;  //极柱温度上限保护值,默认60℃,60-75℃
        //电流保护1%
        uint16_t u16BCurrAlmLimitH;  // 15充放电电流报警,默认105%,0-120%
        uint16_t u16BCurrErrLimitH;  //充放电电流故障,默认110%,0-120%
        uint16_t u16BCurrShorLimit;  //短路电流,默认额10%,范围：0-120%
        //簇电压0.1V
        uint16_t u16BVoltAlmLimitH;  // 18簇高压报警,默认766.8V,0-1000V
        uint16_t u16BVoltErrLimitH;  //簇高压故障,默认788.4V,0-1000V
        uint16_t u16BVoltAlmLimitL;  //簇低压报警,默认604.8V,0-1000V
        uint16_t u16BVoltErrLimitL;  //簇低压故障,默认561.6V,:0-1000V
        //绝缘电阻1K
        uint16_t u16InsResErrLimit;    // 22绝缘过低故障,默认20K，0-50KΩ
        uint16_t u16LeakCurrErrLimit;  //漏电流故障,默认20mA,范围0-50mA
        //保护延时1秒
        uint16_t u16AlarmTimerOutLmt;  // 24报警延时,默认3S,范围:0-180S
        uint16_t u16ErrorTimerOutLmt;  //故障延时(保护),默认1S,0-60S
        //电池参数
        uint16_t u16ClusterStdCap;     // 26簇标称容量0.1kWh,默认224,0-1000
        uint16_t u162ClusterCoeCap;    //簇校正容量0.1kWh,155kWh,0-1000kWh
        uint16_t u16ClusterRemainCap;  //簇电池剩余容量0.1kWh,0-1000kWh
        uint16_t u16ClusterRateCurr;   //簇额定电流0.1A,默认120A,0-600A

        uint16_t u16CurrSensorRange;  // 30电流传感器量程1A,默认200A
        uint16_t u16LeakSensorRange;  //漏电流传感器量程1mA,默认100mA
        uint16_t u16VoltSensorRange;  //电压传感器量程1V,1000V,100-3000

        uint16_t u16BalanceMode;     // 33均衡控制模式
        uint16_t u16BalOnVoltLimit;  //均衡启动电压,3.4V,3.20-3.550V
        uint16_t u16BalOnVoltDiff;   //均衡启动电压差值,30mV,5-500mV

        /* 以下为系统配置参数,由开发/维护人员修改---------------------------------*/
        uint16_t u16ClusterBmuNum;  // 36BMU数量1个,默认18个,范围:1-60个
        uint16_t u16BmuCellNum;     //每个BMU单体电池数量
        uint16_t u16BmuPackTNum;    //每个BMU模组温度个数
        uint16_t u16BmuPoleTNum;    //每个BMU极柱温度个数
        uint16_t u16AlarmMask;      // 40报警屏蔽,默认0,0:不使用1:使用
        uint16_t u16FaultMask;      //故障屏蔽,默认0,0:不使用1:使用
        uint16_t uFunCtrReg;        //使能(电流/电压/漏电/绝缘/双CAN等)

        uint16_t u16LocalIPL;     // 43本地IP低位,192.168 0xa8c0
        uint16_t u16LocalIPH;     //本地IP高位,默认1.120 0x7801
        uint16_t u16TftpServIPL;  // TFTP服务器地址低位192.168 0xA8C0
        uint16_t u16TftpServIPH;  // TFTP服务器地址高位1.230 0xE601
    } Name;
} ST_SysPara;

class mb_cmu : public QThread {
    Q_OBJECT
   protected:
    void run();
    void DealCMD(TMsgData &Msg);

   public:
    mb_cmu();
    ~mb_cmu();
    int Init();  //初始化
    int ReadALL();                     //
    int Close();                    //释放资源
   public:
    uint16_t tab_reg[1000];
    CMU_CONF config;
    vector<MB_CMD> tab_config;
    uint32_t cmu_ver;
    uint32_t cmu_status;
    bool stop;
    int max_offset;
    MessageQueue* pMq;

   private:
    modbus_t* cmu;
    string mb_ip;
    int mb_port;
    int ReadData(uint8_t type, int start, int len, uint16_t* dest);
};

#endif  // MB_CMU_H
