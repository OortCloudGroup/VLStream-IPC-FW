#include "../libpthread/libpthread.h"
#include "../libutility/libutility.h"

#include "t_rtsp_server.h"
#include "../librtsp/rtsp_demo.h"

#ifdef __linux__
#include <unistd.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#define usleep(x) Sleep((x)/1000)
#endif

#define RTSP_THREAD_ALIVE_THRESHOLD				(60 * 100)
#define RTSP_THREAD_PRIORITY						4

THREAD_CONTEXT g_rtsp_thread_context;
static void * rtsp_thread_function(void * arg);

bool t_create_rtsp_thread()
{
	bool ret;
	g_rtsp_thread_context.alive_threshold = RTSP_THREAD_ALIVE_THRESHOLD;
	g_rtsp_thread_context.handler = rtsp_thread_function;
	g_rtsp_thread_context.priority = RTSP_THREAD_PRIORITY;
	if (! (ret = create_thread(&g_rtsp_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}

static void * rtsp_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;
	if (c_init_rtsp() != 0)
	{
		printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
	}
	
	while (! c->exit)
	{
		c->tick = get_tickcount();
		
		// if (c->exit)
		// {
		// 	if (! c_is_server_busy())
		// 		break;
		// }

		if(c_do_rtsp_handler() == 0)
		{
			err("c_do_rtsp_handler stop");
			break;	
		}
	}
	
	c_deinit_rtsp();
	printf("rtsp quited\n");
	c->quited = true;
	
	return NULL;
}