#include "../libpthread/libpthread.h"
#include "../libutility/libutility.h"
#include "../libupload/src/server.h"
#include "../libupload/src/sandbox.h"

#include "../libupload/src/c_upload.h"

#include "vls_upload.h"

#define UPLOAD_THREAD_ALIVE_THRESHOLD				(60 * 100)
#define UPLOAD_THREAD_PRIORITY						4

THREAD_CONTEXT g_upload_thread_context;
static void * upload_thread_function(void * arg);

bool vls_create_upload_thread()
{
	bool ret;
	g_upload_thread_context.alive_threshold = UPLOAD_THREAD_ALIVE_THRESHOLD;
	g_upload_thread_context.handler = upload_thread_function;
	g_upload_thread_context.priority = UPLOAD_THREAD_PRIORITY;
	if (! (ret = create_thread(&g_upload_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}

static void * upload_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;

	if (c_init_upload() != 0)
	{
		printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
	}
	
	while (1)
	{
		//printf("server: %ld \r\n",c->tick);
		//c->tick = get_tickcount();
		//里面是一条下载线程poll的方式阻塞降低cpu占用
		c_do_upload_handler();
	}
	
	c_deinit_upload();
	printf("upload quited\n");
	c->quited = true;
	
	return NULL;
}
