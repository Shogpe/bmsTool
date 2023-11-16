#ifndef _MB_TCP_H_
#define _MB_TCP_H_

#include <QThread>
#include <iostream>
#include "modbus-tcp.h"
#include "modbus-version.h"
#include "modbus.h"
#include "node_conf.h"
#include "db_manager.h"

using namespace std;

class mb_tcp : public QObject {
    Q_OBJECT
    enum _ST_MB {
        _ST_NONE = 0,
        _ST_ONLINE,
    };

   public:
    mb_tcp(QString ip, uint16_t port, uint8_t addr = 1);
    ~mb_tcp();
    int init_config(vector<MB_NODE> &tab_config);  //初始化
    int init_config(vector<db_manager::ST_DB_NODE>& tab_config);
    vector<ST_NODE_DATA> ReadALL();                //
    int write_ao(uint16_t addr, uint16_t len, uint16_t *pv);
    int write_ao(uint16_t addr, uint16_t v);
    int read_value(uint16_t type, uint16_t addr, uint16_t len, uint16_t *v);
    QString get_error_msg();
    void close();   //释放资源
    int Connect();  //释放资源
   public:
    vector<ST_NODE_DATA> tab_data;
    uint32_t drv_status;
    int max_offset;
    map<string, NodeReg> name_map;
    int ReadData(uint8_t type, int start, int len, uint16_t *dest);

   protected:
    modbus_t *cmu;
    int err_counter = 0;
    string mb_ip;
    uint16_t mb_port;
    uint8_t mb_addr;
    int max_frame_len = 120;
    vector<DataReg> reg_list_;  //读取表
    vector<NodeReg> wr_list_;   //下发表
    int ReadAI();
    int JudgeReg(NodeReg &node_reg);
    void NewReg(NodeReg &node_reg);
    void InsertReg(NodeReg &node_reg, int index);
    int ParseData();
   signals:
    void signal_message(const QString &msg);
};

#endif  // _MB_TCP_H_
