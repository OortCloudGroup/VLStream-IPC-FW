#ifndef _T_SERVER_H_
#define _T_SERVER_H_

extern THREAD_CONTEXT g_server_thread_context;

bool t_create_server_thread();

#define t_end_server_thread() end_thread(g_server_thread_context)

#define t_is_server_thread_quited() is_thread_quited(g_server_thread_context)

static inline bool t_is_server_thread_alive()
{
	if (! is_thread_alive(&g_server_thread_context))
	{
		printf("%s: failed\n", __func__);
		return false;
	}
	
	return true;
}

#endif

