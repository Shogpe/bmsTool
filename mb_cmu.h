#ifndef MB_CMU_H
#define MB_CMU_H

#include <QThread>
#include <iostream>

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
    uint8_t type;     // bmu个数
    uint16_t start_addr;     // bmu电压个数
    uint16_t reg_len;    // bmu温度个数
    uint16_t tab_offset;  // bmu状态个数
} MB_CMD;
typedef struct {
    MB_CMD volatge;
    MB_CMD temprature;
    MB_CMD status;
    MB_CMD version;
    MB_CMD other;
} CMU_TABLE_CONF;
#define CMU_ONLINE 0
class mb_cmu : public QThread {
    Q_OBJECT
   protected:
    void run();

   public:
    mb_cmu();
    //  mb_cmu(string ip, int port);
    ~mb_cmu();
    int Init(string ip, int port);  //初始化
    int Loop();                     //
    int Close();                    //释放资源
   public:
    uint16_t tab_reg[1000];
    CMU_CONF config;
    MB_CMD tab_config[10];
    uint32_t cmu_ver;
    uint32_t cmu_status;
    bool stop;

   private:
    modbus_t* cmu;
    string mb_ip;
    int mb_port;
    int ReadData(uint8_t type, int start, int len, uint16_t* dest);
};

#endif  // MB_CMU_H
