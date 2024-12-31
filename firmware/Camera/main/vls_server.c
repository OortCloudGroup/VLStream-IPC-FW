#include "../libpthread/libpthread.h"
#include "../libutility/libutility.h"
#include "../libhttpserver/common.h"
#include "../libhttpserver/c_server.h"

#include "vls_server.h"

#define SERVER_THREAD_ALIVE_THRESHOLD				(60 * 100)
#define SERVER_THREAD_PRIORITY						4

THREAD_CONTEXT g_server_thread_context;
static void * server_thread_function(void * arg);

bool vls_create_server_thread()
{
	bool ret;
	g_server_thread_context.alive_threshold = SERVER_THREAD_ALIVE_THRESHOLD;
	g_server_thread_context.handler = server_thread_function;
	g_server_thread_context.priority = SERVER_THREAD_PRIORITY;
	if (! (ret = create_thread(&g_server_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}

static void * server_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;

	if (! c_init_server())
	{
		printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
	}
	
	while (1)
	{
		c->tick = get_tickcount();
		
		if (c->exit)
		{
			if (! c_is_server_busy())
				break;
		}

		c_do_server_handler();
	}
	
	c_deinit_server();
	printf("server quited\n");
	c->quited = true;
	
	return NULL;
}
