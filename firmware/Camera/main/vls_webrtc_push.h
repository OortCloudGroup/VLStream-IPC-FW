#ifndef _VLS_WEBRTC_PUSH_H_
#define _VLS_WEBRTC_PUSH_H_

extern THREAD_CONTEXT g_wertc_thread_context;

bool vls_create_webrtc_thread();

#define vls_end_webrtc_thread() end_thread(g_wertc_thread_context)

#define vls_is_webrtc_thread_quited() is_thread_quited(g_wertc_thread_context)

static inline bool vls_is_webrtc_thread_alive()
{
	if (! is_thread_alive(&g_wertc_thread_context))
	{
		printf("%s: failed\n", __func__);
		return false;
	}
	
	return true;
}

#endif

