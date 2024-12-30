#ifndef _G_UTILITY_1H_
#define _G_UTILITY_1H_
#ifdef __cplusplus
extern "C"{
#endif

#ifdef MAX
#undef MAX
#endif
#define MAX(x,y) (((x)<(y))?(y):(x))

#include "g_datatypes.h"
#include <linux/tcp.h>

#define my_malloc(size) malloc(size)
#define my_realloc(p, size) realloc(p, size)
#define my_free(p) free(p)
#define my_safe_free(a) { if (a) {my_free(a); a = NULL;} }
#define my_safe_strlen(x) ((x == NULL)?0:strlen(x))
#define get_tickcount() (unsigned long)times(NULL)
#define TICKS_OF_DAYS(days) (days * 24 * 60 * 60 * sysconf(_SC_CLK_TCK))
#define TICKS_OF_HOURS(hours) (hours * 60 * 60 * sysconf(_SC_CLK_TCK))
#define TICKS_OF_MINUTES(minutes) (minutes * 60 * sysconf(_SC_CLK_TCK))
#define TICKS_OF_SECONDS(seconds) (seconds * sysconf(_SC_CLK_TCK))
#define TICKS_OF_MS(ms) (ms * sysconf(_SC_CLK_TCK) / 1000)

//----------------------------------------------------
//	socket
static inline void set_socket_timeout(int s, unsigned int w_timeout, unsigned int r_timeout)
{
	struct timeval tv;
	tv.tv_sec = w_timeout;
	tv.tv_usec = 0;
	setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv));
	tv.tv_sec = r_timeout;
	setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv));
}

static inline void set_socket_buff_size(int s, unsigned int w_size, unsigned int r_size)
{
	setsockopt(s,SOL_SOCKET,SO_SNDBUF,(char *)&w_size,sizeof(int));
	setsockopt(s,SOL_SOCKET,SO_RCVBUF,(char *)&r_size,sizeof(int));
}

static inline void disable_socket_linger(int s)
{
	struct linger linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(s,SOL_SOCKET, SO_LINGER,(const char *)&linger,sizeof(linger));
}

static inline void set_socket_reuseaddr(int s)
{
	int opt = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
}

static inline void set_socket_broadcast(int s)
{
	int opt = 1;
	setsockopt(s,SOL_SOCKET,SO_BROADCAST,(char *)&opt,sizeof(int));
}

static inline void set_socket_nodelay(int s)
{
	int opt = 1;
	int ret = -1;
	ret = setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof(int));

	if (ret < 0)
	{
		printf("Couldn't setsockopt(TCP_NODELAY).\n");
	}
}

static inline void set_socket_nonblock(int s)
{
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);
}

static inline void set_socket_ttl(int s, unsigned int ttl)
{
	setsockopt(s,IPPROTO_IP,IP_TTL,(char *)&ttl,sizeof(int));
}

static inline void set_socket_multicast(int s, unsigned char ttl, unsigned long ip)
{
	setsockopt(s,IPPROTO_IP,IP_MULTICAST_TTL,(char *)&ttl,sizeof(ttl));
	setsockopt(s,IPPROTO_IP,IP_MULTICAST_IF,(char *)&ip,sizeof(int));
}

bool is_socket_writeable(int s, unsigned int ms);
bool is_socket_readable(int s, unsigned int ms);
bool socket_connect(int s, const char * host, unsigned short port, unsigned int timeout);
bool socket_send(int s, const char * data, unsigned int len);
bool socket_recv(int s, char * data, unsigned int len);
unsigned long my_gethostbyname(const char * server);
char * get_local_ip_addr(const char * if_name);
int dns_client(const char * server);
//----------------------------------------------------//----------------------------------------------------
//	string
static inline unsigned char * mem_new(int value, unsigned int len)
{
	unsigned char * p = (unsigned char *)my_malloc(len);
	if (! p)
		return NULL;
	memset(p, value, len);
	return p;
}

static inline unsigned char * mem_assign(const unsigned char * value, unsigned int len)
{
	unsigned char * p = (unsigned char *)my_malloc(len);
	if (! p)
		return NULL;
	memcpy(p, value, len);
return p;
}

static inline unsigned char * mem_combine(const unsigned char * src1, unsigned int src1_len, const unsigned char * src2, unsigned int src2_len)
{
	unsigned char * p = (unsigned char *)my_malloc(src1_len + src2_len);
	if (! p)
		return NULL;
	memcpy(p, src1, src1_len);
	memcpy(p + src1_len, src2, src2_len);
	return p;
}

static inline unsigned char * mem_append(unsigned char * src, unsigned int src_len, const unsigned char * append, unsigned int append_len)
{
	unsigned char * p = mem_combine(src, src_len, append, append_len);
	my_free(src);
	return p;
}

static inline char * str_assign(const char * value)
{
	return (char *)mem_assign((unsigned char *)value, strlen(value) + 1);
}

static inline char * str_append(char * src, const char * append)
{
	return (char *)mem_append((unsigned char *)src, strlen(src), (const unsigned char *)append, strlen(append) + 1);
}

static inline char * str_combine(const char * src1, const char * src2)
{
	return (char *)mem_combine((const unsigned char *)src1, strlen(src1), (const unsigned char *)src2, strlen(src2) + 1);
}

#define JS								0
#define JSON							1
#define JSONP							2

//	thread
#define MIN_PRIORITY					0
#define MAX_PRIORITY					5
typedef struct tagThreadContext
{
	unsigned long alive_threshold;
	int priority;
	unsigned long tick;
	bool exit;
	bool quited;
	pthread_t thread;
	void * (* handler)(void * arg);
} THREAD_CONTEXT;
bool create_thread(THREAD_CONTEXT * c);
#define end_thread(c) c.exit = true
bool is_thread_alive(THREAD_CONTEXT * c);
#define is_thread_quited(c) c.quited
void set_thread_priority(int priority);


//----------------------------------------------------
//	msg_queue

#define MSG_MAGIC		0x3db0aa74

typedef struct tagMsg
{
	unsigned int magic;
	unsigned int msg;
	unsigned int param1;
	unsigned int param2;
	unsigned int param3;
	unsigned int param4;
} MSG;

typedef struct tagMsgConext
{
	int s_w;
	int s_r;
	pthread_mutex_t mutex;
	char buffer[sizeof(MSG)];
	unsigned int len;
} MSG_CONTEXT;


void destroy_msg_queue(MSG_CONTEXT * c);
bool recv_msg(MSG_CONTEXT * c, MSG * msg);
bool send_msg(MSG_CONTEXT * c, MSG * msg);
bool create_msg_queue(MSG_CONTEXT * c, unsigned int size);

#ifdef __cplusplus
}
#endif
#endif
