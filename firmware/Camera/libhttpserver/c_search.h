#ifndef _C_SEARCH_H_
#define _C_SEARCH_H_
#ifdef __cplusplus
extern "C"{
#endif
#define WM_USER                             0x0400
#define WM_SEARCH						(WM_USER + 4100)

typedef struct tagSearchParam
{
	void * client;
	char * key;
	char * page;
	char * numline;
} SEARCH_PARAM;

#ifdef __cplusplus
}
#endif
#endif
