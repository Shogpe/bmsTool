#ifndef CRC32_H_
#define CRC32_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define CRC_BUFFER_SIZE 8192
#ifdef __cplusplus
extern "C" {
#endif
int           crc32_file( FILE* file, unsigned long* outCrc32 );
unsigned long crc32_buf( unsigned long inCrc32, const void* buf,
                         size_t bufLen );
uint32_t      CRC32Stm32( uint32_t* pData, uint16_t Length );
#ifdef __cplusplus
}
#endif
#endif
