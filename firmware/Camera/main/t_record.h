#ifndef _T_RECORD_H_
#define _T_RECORD_H_

#include "c_queue_frame.h"

extern THREAD_CONTEXT g_record_thread_context;
//extern FrameQueue g_queue;

bool t_create_record_thread();

#define t_end_record_thread() end_thread(g_record_thread_context)

#define t_is_record_thread_quited() is_thread_quited(g_record_thread_context)

static inline bool t_is_record_thread_alive()
{
	if (! is_thread_alive(&g_record_thread_context))
	{
		printf("%s: failed\n", __func__);
		return false;
	}
	
	return true;
}

	
#endif