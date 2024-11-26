#ifndef NODE_CONF_H
#define NODE_CONF_H
#include <iostream>
#include <vector>
using  std::vector;
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#ifdef __cplusplus
extern "C" {
#endif
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
typedef struct structReg {
    int dev_id;                  //设备地址
    unsigned char reg_type;      //寄存器类型，功能码
    int reg_start;               //起始地址
    int reg_num;                 //连续个数
    int data_num;                //数据个数
    vector<DatabaseIO> data_io;  //实时数据库,连续个数
} DataReg;

#define IO_MAX 200

#define NONE_REG 0x00
#define DO_REG   0x01
#define DI_REG   0x02
#define AO_REG   0x03
#define AI_REG   0x04

//写入结构体
typedef struct structTable {
    uint32_t index;          //数据库序号
    unsigned char reg_type;  //寄存器类型，功能码
    int reg_addr;            //寄存器地址
    int data_type;           //数据类型
    double default_val;      //初值
    float factor;            //变比
} NodeReg;
typedef struct {
    int index;           // 数据索引
    char name[64];       // 控件名
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

extern MB_NODE cmu_v4_config[];
extern MB_NODE cmu_v4_10config[];
extern MB_NODE cmu_v4_6config[];
extern MB_NODE cmu_v5_0config[];
extern int GetCMUConfigArrayLen(MB_NODE arg[]);
//extern const int cmu_v4_config_len;

#ifdef __cplusplus
}
#endif
#endif  // NODE_CONF_H
