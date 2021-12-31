#ifndef NODE_CONF_H
#define NODE_CONF_H
#include "mb_tcp.h"
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#ifdef __cplusplus
extern "C" {
#endif

extern MB_NODE cmu_v1_config[];
extern const int cmu_v1_config_len;
extern MB_NODE cmu_v2_config[];
extern const int cmu_v2_config_len;
extern MB_NODE cmu_v3_config[];
extern const int cmu_v3_config_len;
extern MB_NODE cmu_v4_config[];
extern const int cmu_v4_config_len;
#ifdef __cplusplus
}
#endif
#endif  // NODE_CONF_H
