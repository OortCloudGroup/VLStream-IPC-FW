// #ifndef _VLS_RECORD_H_
// #define _VLS_RECORD_H_

// #include "c_queue_frame.h"

// extern THREAD_CONTEXT g_record_thread_context;
// //extern FrameQueue g_queue;

// bool vls_create_record_thread();

// #define vls_end_record_thread() end_thread(g_record_thread_context)

// #define vls_is_record_thread_quited() is_thread_quited(g_record_thread_context)

// static inline bool vls_is_record_thread_alive()
// {
// 	if (! is_thread_alive(&g_record_thread_context))
// 	{
// 		printf("%s: failed\n", __func__);
// 		return false;
// 	}
	
// 	return true;
// }

	
// #endif