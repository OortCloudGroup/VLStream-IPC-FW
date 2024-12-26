#ifndef _T_RTSP_SERVER_H_
#define _T_RTSP_SERVER_H_

extern THREAD_CONTEXT g_rtsp_thread_context;
//extern FrameQueue g_queue;

bool t_create_rtsp_thread();

#define t_end_rtsp_thread() end_thread(g_rtsp_thread_context)

#define t_is_rtsp_thread_quited() is_thread_quited(g_rtsp_thread_context)

static inline bool t_is_rtsp_thread_alive()
{
	if (! is_thread_alive(&g_rtsp_thread_context))
	{
		printf("%s: failed\n", __func__);
		return false;
	}
	
	return true;
}

#endif