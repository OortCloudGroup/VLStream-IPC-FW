#ifndef _G_HTTP_H_
#define _G_HTTP_H_
#ifdef __cplusplus
extern "C"{
#endif

#include "g_datatypes.h"
typedef enum {
	GET = 0, POST
} HTTP_METHOD;

typedef struct tagHttpParam
{
	char * name;
	char * value;
	struct list_head list;
} HTTP_PARAM;

typedef struct tagHttpRequestContext
{
	const char * home;
	const char * default_file;
	
	char * data;
	unsigned int len;
	unsigned int total;
	
	bool first_line_ok;
	HTTP_METHOD method;
	char * absolute_path;
	char * relative_path;
	char * user;
	char * pwd;
	unsigned int content_len;
	unsigned int boundary_len;
	char  boundary[100];
	int json;
	char * jsonp_callback;
	char * next_url;
	char * realm_str;
	struct list_head params_list;
} HTTP_REQUEST_CONTEXT;
static inline void http_request_open(HTTP_REQUEST_CONTEXT * c, const char * home, const char * default_file)
{
	memset(c, 0, sizeof(HTTP_REQUEST_CONTEXT));
	c->home = home;
	c->default_file = default_file;
	INIT_LIST_HEAD(&c->params_list);
}
void http_request_close(HTTP_REQUEST_CONTEXT * c);

#define HTTP_OK								1
#define HTTP_CONTINUE						0
#define HTTP_INTERNAL_ERROR					-1
#define HTTP_BAD_REQUEST					-2
int http_request_parse(HTTP_REQUEST_CONTEXT * c, char * data, unsigned int len);
char * http_request_get_param(HTTP_REQUEST_CONTEXT * c, const char * name);
#ifdef __cplusplus
}
#endif
#endif
