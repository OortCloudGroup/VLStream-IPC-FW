#ifndef _VLS_SERVER_H_
#define _VLS_SERVER_H_

extern THREAD_CONTEXT g_server_thread_context;

bool vls_create_server_thread();

#define vls_end_server_thread() end_thread(g_server_thread_context)

#define vls_is_server_thread_quited() is_thread_quited(g_server_thread_context)

static inline bool vls_is_server_thread_alive()
{
	if (! is_thread_alive(&g_server_thread_context))
	{
		printf("%s: failed\n", __func__);
		return false;
	}
	
	return true;
}

#endif

