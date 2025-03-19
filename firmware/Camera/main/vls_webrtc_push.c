#include "../libpthread/libpthread.h"
#include "../libutility/libutility.h"

// #include "../libwebrtc/src/recordpushrtc.h"
#include "../libwebrtc/webrtc/src/recordpushrtc_capi.h"
#include "vls_webrtc_push.h"

#define WEBRTC_THREAD_ALIVE_THRESHOLD				(60 * 100)
#define WEBRTC_THREAD_PRIORITY						4

THREAD_CONTEXT g_webrtc_thread_context;
static void * webrtc_thread_function(void * arg);

bool vls_create_webrtc_thread()
{
	bool ret;
	g_webrtc_thread_context.alive_threshold = WEBRTC_THREAD_ALIVE_THRESHOLD;
	g_webrtc_thread_context.handler = webrtc_thread_function;
	g_webrtc_thread_context.priority = WEBRTC_THREAD_PRIORITY;
	if (! (ret = create_thread(&g_webrtc_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}

static void * webrtc_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;

	// if (c_init_wertc() != 0)
	// {
	// 	printf("%s: reboot\n", __func__);
	// 	//c_reboot();
	// 	return NULL;
	// }
	  // 创建对象
	void* recorder = record_push_create();
	 //初始化参数
	record_push_init(recorder);
	//消息队列初始化
	record_message_init(recorder);
	while (1)
	{
		
		start_push_camera(recorder,"http://192.168.246.131:1985/rtc/v1/whip/?app=live&stream=livestream");
		while (1)
		{
			/* code */
		}
		
	}
	
	// 销毁对象
    record_push_destroy(recorder);
	printf("wertc quited\n");
	c->quited = true;
	
	return NULL;
}
