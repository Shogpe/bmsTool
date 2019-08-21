#ifndef MB_CMU_H
#define MB_CMU_H

#include <QThread>
#include <iostream>

#include "modbus-tcp.h"
#include "modbus-version.h"
#include "modbus.h"
#include "MessageQueue.h"
using namespace std;
typedef struct {
    uint16_t bmu_num;     // bmu个数
    uint16_t vol_num;     // bmu电压个数
    uint16_t temp_num;    // bmu温度个数
    uint16_t status_num;  // bmu状态个数
} CMU_CONF;

typedef struct {
    uint8_t type;     // 寄存器类型
    uint16_t start_addr;     // 寄存器起始地址
    uint16_t reg_len;    //寄存器长度
    uint16_t tab_offset;  // 转存表偏移
} MB_CMD;

typedef struct {
    int index;     // 数据索引
    uint32_t data_type;     // 数据类型
    float factor;    //变比
    string name;  // 控件名
} MB_NODE;

typedef enum {
    NONE = 0,
    THREAD_START,   //
    THREAD_STOP,    //线程退出


} MSG_TYPE;
#define CMU_ONLINE 0
#define CMU_OUTOFDATE 31

#define TAB_SYS_LEN 12  //系统数据:时钟,状态
#define TAB_ENG_LEN 42  //能量数据:SOC,电量
#define TAB_CFG_LEN 42  //配置数据:参数
#define TAB_CMU_LEN 25  //统计数据:计算极值

class mb_cmu : public QThread {
    Q_OBJECT
   protected:
    void run();

   public:
    mb_cmu();
    ~mb_cmu();
    int Init(string ip, int port);  //初始化
    int Loop();                     //
    int Close();                    //释放资源
   public:
    uint16_t tab_reg[1000];
    CMU_CONF config;
    vector<MB_CMD> tab_config;
    uint32_t cmu_ver;
    uint32_t cmu_status;
    bool stop;
    int max_offset;
    MessageQueue *pMq;
   private:
    modbus_t* cmu;
    string mb_ip;
    int mb_port;
    int ReadData(uint8_t type, int start, int len, uint16_t* dest);
};

#endif  // MB_CMU_H
