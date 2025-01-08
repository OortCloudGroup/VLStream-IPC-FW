#ifndef _VLS_UPLOAD_H_
#define _VLS_UPLOAD_H_

extern THREAD_CONTEXT g_upload_thread_context;

bool vls_create_upload_thread();

#define vls_end_upload_thread() end_thread(g_upload_thread_context)

#define vls_is_upload_thread_quited() is_thread_quited(g_upload_thread_context)

static inline bool vls_is_upload_thread_alive()
{
	if (! is_thread_alive(&g_upload_thread_context))
	{
		printf("%s: failed\n", __func__);
		return false;
	}
	
	return true;
}

#endif

