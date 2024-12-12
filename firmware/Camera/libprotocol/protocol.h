/*
 Authors: ZhangXuelian
 	


 Changes:
 	
	
*/

#ifndef _G_PROTOCOL_H_
#define _G_PROTOCOL_H_
#ifdef __cplusplus
extern "C"{
#endif

// ��Ҫ�����˽��Э�飬�������
// 1��Ҫ֧�ֵ�˽��Э��Ŀͻ�������
typedef enum {
	UNKNOWN_CLIENT = 0,
	HTTP_CLIENT,
	COMMAND_CLIENT,
	LOG_CLIENT,
} CLIENT_TYPE;


// 2��Ҫ֧�ֵ�˽��Э���ͷ�ļ���
// protocol public file
#include "cmd_proto.h"
bool parse_command_recved_data(void * __client);

// comm function.

#ifdef __cplusplus
}
#endif
#endif
