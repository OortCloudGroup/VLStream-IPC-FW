#ifndef _G_DATATYPES_H
#define _G_DATATYPES_H
#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#ifndef __cplusplus 
typedef enum {false = 0, true = !false} bool;
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef uint64_t u64;


#define	ERROR_OK									0
#define ERROR_UNKNOWN								-1
#define ERROR_NO_IMPLEMENT							-2
#define ERROR_INTERNAL								-3
#define ERROR_BLOCKED								-4
#define ERROR_OUT_OF_RANGE							-5
#define	ERROR_BAD_PARAM								-6
#define	ERROR_BAD_STATUS							-7
#define	ERROR_BAD_ID								-8
#define ERROR_BAD_PACKET							-9
#define ERROR_CONN_ABORTED							-10
#define ERROR_CONN_CLOSED							-11
#define ERROR_CONN_FAILED							-12
#define ERROR_CONN_TIMEOUT							-13
#define	ERROR_DEVICE_BAD_AUTH						-14
#define ERROR_DEVICE_TOO_MANY_SESSIONS				-15
#define ERROR_DEVICE_INTERNAL						-16
#define ERROR_DEVICE_BAD_PARAM						-17		
#define ERROR_DEVICE_FORBIDDEN						-18
#define ERROR_DEVICE_OPERATION_FAIL					-19
#define ERROR_DEVICE_BAD_STATUS						-20
#define ERROR_DEVICE_UNKNOWN						-21
#define ERROR_REQ_TIMEOUT							-22
#define ERROR_DOUBLE_REQUEST						-23

#define MAX_PATH          260

#ifdef __cplusplus
}
#endif
#endif
