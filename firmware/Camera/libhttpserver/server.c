
#include "g_datatypes.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/timex.h>	//	for ntpc
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <signal.h>
#include <linux/types.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <net/if_arp.h>
#include <sys/soundcard.h>

#include "g_list.h"
#include "g_utility.h"
#include "g_http.h"

#include <netinet/ip.h>      // for icmp_ping
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include "c_server.h"
#include "c_search.h"
#include "g_cJSON.h"

/////////////serarch index//////////////
#include "common.h"

extern INDEX_CONTEXT index_context;
#define T__INDEX_CONTEXT (&(index_context))
//////////////////////

#define WEBSERVER_NAME					     			"uhttpd/0.0.010"
#define HTTP_HOME						                "/web"
#define HTTP_DEFAULT_FILE								"/index.htm"
#define MIN_HTTP_SEND_BUFFER_SIZE						1024

#define SERVER_MSG_QUEUE_SIZE							1024
#define SERVER_BUFFER_SIZE								(1024 * 200)
#define MAX_CLIENTS_NUMBER								(FD_SETSIZE - 2)
#define VIDEOSTREAM_BOUNDARY							"wifi_shd"
#define RFC1123FMT										"%a, %d %b %Y %H:%M:%S GMT"


#define	JSON_ERROR_OK					0
#define	JSON_ERROR_BAD_AUTH				-1
#define	JSON_ERROR_BAD_PARAM			-2
#define	JSON_ERROR_MAX_SESSIONS			-3
#define	JSON_ERROR_INTERNAL				-4
#define	JSON_ERROR_TIMEOUT				-5
#define	JSON_ERROR_BAD_REQUEST			-6
#define	JSON_ERROR_SYSTEM_BUSY			-7


// server
typedef struct tagServerContext
{
	int listen_sock;
	MSG_CONTEXT msg_queue;
	struct list_head clients_list;
	unsigned int clients_number;
	fd_set w_fds;
	fd_set r_fds;
	char buffer[SERVER_BUFFER_SIZE];  
	char time_buffer[64];
	unsigned long tick;
	bool first_line_ok;
} SERVER_CONTEXT;

//////////////////////////////////////////////////////////////////////////
// client
typedef enum {
	UNKNOWN_CLIENT = 0,
	HTTP_CLIENT,
	BINARY_CLIENT,
	LOCAL_CLIENT,
} CLIENT_TYPE;

typedef enum {
	RECV_HTTP_HEADER = 0,
	RECV_HTTP_DATA,
	SEND_HTTP_RESULT,
	WAIT_HTML_RESULT,
	WAIT_HTTP_CLOSE
} HTTP_PERIOD;

typedef struct tagClient
{
	int sdk_session; 
	CLIENT_TYPE type;    
	int sock;           
	unsigned char temp_buffer_4_unknown_client[4];
	int len_of_temp_data_4_unknown_client;         

	unsigned long tick; 
	unsigned long peer_ip;
	bool busy;               
	struct list_head list;   

	HTTP_REQUEST_CONTEXT hrc; 
	HTTP_PERIOD http_period;  
	FILE * http_file;
	struct list_head files_get_task_list;
	struct list_head file_put_task_list;
	struct list_head open_file_task_list; 

	char * send_data;
	unsigned int send_data_offset;
	unsigned int send_data_len;
	char * recv_data;             
	unsigned int recv_data_len;
	unsigned long binary_r_tick;
	unsigned long binary_w_tick;
} CLIENT;


SERVER_CONTEXT g_server_context = {0};
#include "g_utility.h"

//extern SERVER_CONTEXT g_server_context;

extern bool _make_http_client_resp(CLIENT * client, int result, bool reauth, const char * title, const char * content_type, const char * content, int content_len);
#define c_make_http_client_resp(client, result, title, content) _make_http_client_resp(client, result, false, title, "text/plain", content, strlen(content))
#define c_make_http_client_html_resp(client, content, content_len) _make_http_client_resp(client, 200, false, "OK", "text/plain", content, content_len)
bool c_make_http_client_json_error_resp(CLIENT * client, int error);

bool _make_http_client_resp(CLIENT * client, int result, bool reauth, const char * title, const char * content_type, const char * content, int content_len)
{
	printf("%s: Enter!\n", __func__);
	SERVER_CONTEXT * c = &g_server_context;
	time_t now;

	if (NULL == (client->send_data = (char *)my_malloc(content_len + MIN_HTTP_SEND_BUFFER_SIZE)))
		return false;
	time(&now);
	strftime(c->time_buffer, 64, RFC1123FMT, gmtime(&now));
	if (reauth)
	{
		char nonce[33];
		client->send_data_len = sprintf(client->send_data,
			"HTTP/1.1 %d %s\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Access-Control-Allow-Methods: POST, GET\r\n"
			"Server: "WEBSERVER_NAME"\r\n"
			"Date: %s\r\nWWW-Authenticate: Digest realm=\"netac wifi shd\","
			"qop=\"auth\","
			"algorithm=MD5,"
			"nonce=\"%s\"\r\n" 
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n"
			"Cache-Control: no-cache\r\n"
			"Connection: close\r\n\r\n%s", 
			result, title, c->time_buffer, nonce, content_type, content_len, content);
	}
	else
	{
		client->send_data_len = sprintf(client->send_data, 
			"HTTP/1.1 %d %s\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"Access-Control-Allow-Methods: POST, GET\r\n"
			"Server: "WEBSERVER_NAME"\r\n"
			"Date: %s\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n"
			"Cache-Control: no-cache\r\n"
			"Connection: close\r\n\r\n%s", 
			result, title, c->time_buffer, content_type, content_len, content);
	}
	client->send_data_offset = 0;
	client->tick = c->tick;
	client->http_period = SEND_HTTP_RESULT;
	printf("%s: Leave!\n", __func__);
	return true;	
}

bool c_make_http_client_json_error_resp(CLIENT * client, int error)
{
	SERVER_CONTEXT * c = &g_server_context;
	int len;

	if (client->hrc.json == JSONP)
		len = sprintf(c->buffer, "%s({\"error\":%d});", ((! client->hrc.jsonp_callback)||(! client->hrc.jsonp_callback[0]))?"callback":client->hrc.jsonp_callback, error);
	else
		len = sprintf(c->buffer, "{\"error\":%d}", error);
	return c_make_http_client_html_resp(client, c->buffer, len);
}

static inline bool make_http_client_file_resp(CLIENT * client, const char * content_type, const char * filepath, const char * filename);

static inline bool make_http_client_file_resp(CLIENT * client, const char * content_type, const char * filepath, const char * filename)
{
	SERVER_CONTEXT * c = &g_server_context;
	time_t now;

	if (strstr(filepath, ".."))
		return c_make_http_client_resp(client, 200, "Bad Request", "No service for directory.");
	if (NULL == (client->http_file = fopen(filepath, "rb")))
		return c_make_http_client_resp(client, 200, "Not Found", "File not found.");
	if (NULL == (client->send_data = (char *)my_malloc(MIN_HTTP_SEND_BUFFER_SIZE)))
	{
		fclose(client->http_file);
		client->http_file = NULL;
		return c_make_http_client_resp(client, 200, "Internal Server Error", "Not enough memory.");
	}

	time(&now);
	strftime(c->time_buffer, 64, RFC1123FMT, gmtime(&now));
	fseek(client->http_file, 0, SEEK_END);
	if (filename)
		client->send_data_len = sprintf(client->send_data, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET\r\nServer: "WEBSERVER_NAME"\r\nDate: %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\nContent-disposition: attachment; filename=\"%s\"\r\nCache-Control: private\r\nConnection: close\r\n\r\n", c->time_buffer, content_type, ftell(client->http_file), filename);
	else
		client->send_data_len = sprintf(client->send_data, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET\r\nServer: "WEBSERVER_NAME"\r\nDate: %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\nCache-Control: private\r\nConnection: close\r\n\r\n", c->time_buffer, content_type, ftell(client->http_file));
	rewind(client->http_file);
	client->send_data_offset = 0;
	client->tick = c->tick;
	client->http_period = SEND_HTTP_RESULT;
	return true;
}


int c_search(void * client ,char * key)
{
	printf("Enter %s\n", __func__);
	SERVER_CONTEXT * c = &g_server_context;
	SEARCH_PARAM * param;
	int ret = 0;

	
	if (NULL == (param = (SEARCH_PARAM*)my_malloc(sizeof(SEARCH_PARAM))))
		return ERROR_INTERNAL;

	param->key = str_assign(key);
	param->client = client;
	MSG msg;
	msg.msg = WM_SEARCH;
	msg.param1 = (unsigned long)param;
	send_msg(&c->msg_queue, &msg);
quit:
	return ret;
}


void urldecode(char *p)  
{  
	register i=0;  
	while(*(p+i))  
	{  
		if ((*p=*(p+i)) == '%')  
		{  
			*p=*(p+i+1) >= 'A' ? ((*(p+i+1) & 0XDF) - 'A') + 10 : (*(p+i+1) - '0');  
			*p=(*p) * 16;  
			*p+=*(p+i+2) >= 'A' ? ((*(p+i+2) & 0XDF) - 'A') + 10 : (*(p+i+2) - '0');  
			i+=2;  
		}  
		else if (*(p+i)=='+')  
		{  
			*p=' ';  
		}  
		p++;  
	}  
	*p='\0';
	printf("#############key=%s\n",p);
}  

char * del_line_symbol(char **str)
{
	char *p =str;
	int i=0;
	int len=0;
	len = strlen(p);
	for(i=0;i !='0';i++)
	{
		if(p[i] =='\n')
			p[i]=' ';
	}

}


static inline bool handle_search_html_request(CLIENT * client)
{
	SERVER_CONTEXT * c = &g_server_context;
	HTTP_PARAM * http_param;
	struct list_head * pos, * next;
	int content_len = 0;
	int len;

	if (list_empty(&client->hrc.params_list))
	{
		return c_make_http_client_json_error_resp(client, JSON_ERROR_BAD_PARAM);
	}
	else
	{
		char * key = NULL;
		list_for_each_safe(pos, next, &client->hrc.params_list)
		{
			http_param = list_entry(pos, HTTP_PARAM, list);
			list_del(pos);
			if (0 == strcasecmp(http_param->name, "key"))
				key = str_assign(http_param->value);
			my_safe_free(http_param);	
		}

		//urldecode(key);
		
		if (0 > (len = c_search(client,key)))		
			return c_make_http_client_json_error_resp(client, JSON_ERROR_BAD_PARAM);
		content_len += len;
		my_safe_free(key);	
	}
	return true;//c_make_http_client_html_resp(client, c->buffer, content_len);
}


static inline bool handle_http_client_request(CLIENT * client)
{
	SERVER_CONTEXT * c = &g_server_context;
	
	// process html this rest api.
	int i;
	int path_len = strlen(client->hrc.relative_path);
	int suffix_len = strlen(".html");

	if ((path_len >= suffix_len) &&
		(0 == strcasecmp(client->hrc.relative_path + path_len - suffix_len, ".html")))
	{
		if (0 == strcasecmp(client->hrc.relative_path, "/search.html"))
		{
			return handle_search_html_request(client);
			
		}
	}

	// process common files
	// http://www.cnblogs.com/campo/archive/2007/02/02/638348.aspx
	static struct tagMIMEType {
		const char * suffix;
		const char * content_type;
	} mime_types[] = {
		{".jpg", "image/jpeg"}, 
		{".gif", "image/gif"},
		{".png", "image/png"},
		{".htm", "text/html"}, 
		{".js", "text/javascript"}, 
		{".css", "text/css"}, 
		{".cab", "application/octet-stream"},
		{".ico", "image/x-icon"},
		{".exe", "application/octet-stream"},
		{".html", "text/html"}, 
		{".php", "text/html"}, 
		{".shtml", "text/html"}, 
		{".rar", "application/rar"},
		{".zip", "application/zip"},
		{".ocx", "application/octet-stream"},
		{".asf", "video/x-ms-asf"},
		{".wav", "audio/x-wav"},
		{".m3u8", "application/x-mpegURL"},
		{".ts", "video/MP2T"},
		{".tar.gz","application/tar"},
	};

	for (i = 0;i < sizeof(mime_types) / sizeof(struct tagMIMEType);i++)
	{
		suffix_len = strlen(mime_types[i].suffix);
		if ((path_len >= suffix_len) &&
			(0 == strcasecmp(client->hrc.relative_path + path_len - suffix_len, mime_types[i].suffix)))
		{
			return make_http_client_file_resp(client, mime_types[i].content_type, client->hrc.absolute_path, NULL);
		}
	}
	return make_http_client_file_resp(client, "application/octet-stream", client->hrc.absolute_path, NULL);
}


bool c_parse_http_client_recved_data(CLIENT * client, int len)
{
	SERVER_CONTEXT * c = &g_server_context;
	bool ret = true;
	int ret2;

	switch(client->http_period) 
	{
	case RECV_HTTP_HEADER:
		ret2 = http_request_parse(&client->hrc, c->buffer, len);
		if (ret2 == HTTP_BAD_REQUEST)
			ret = c_make_http_client_resp(client, 200, "Bad Request", "Can\'t parse request.");
		else if (ret2 == HTTP_INTERNAL_ERROR)
			ret = c_make_http_client_resp(client, 200, "Internal Server Error", "Not enough memory.");
		else if (ret2 == HTTP_OK)
			ret = handle_http_client_request(client);
		break;
	default:
		break;
	}
	return ret;
}

static inline void deinit_client(CLIENT * client);

static inline void remove_client(CLIENT * client)
{
	SERVER_CONTEXT * c = &g_server_context;
	list_del(&client->list);
	deinit_client(client);
	close(client->sock);
	my_free(client);
	c->clients_number --;
}

static inline void clear_clients()
{
	SERVER_CONTEXT * c = &g_server_context;
	struct list_head * pos, * next;
	CLIENT * client;

	list_for_each_safe(pos, next, &c->clients_list)
	{
		client = list_entry(pos, CLIENT, list);
		remove_client(client);
	}
}

static inline bool start_listen()
{
	SERVER_CONTEXT * c = &g_server_context;
	unsigned short port;
	int https;
	struct sockaddr_in host;

	port = (unsigned short)8080;

	if (0 > (c->listen_sock = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("%s: can not create socket %d\n", __func__, c->listen_sock);
		c->listen_sock = 0;
		goto failed;
	}

	set_socket_reuseaddr(c->listen_sock);
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = 0;
	host.sin_port = htons(port);
	if (0 > bind(c->listen_sock,(struct sockaddr *)&host,sizeof(host)))
	{
		printf("%s: can not listen socket on port %u\n", __func__, port);
		perror("bind failed\n");
		goto failed;
	}
	if (0 > listen(c->listen_sock,SOMAXCONN))
	{
		printf("%s: can not listen\n", __func__);
		goto failed;
	}
	return true;

failed:
	if (c->listen_sock)
	{
		close(c->listen_sock);
		c->listen_sock = 0;
	}
	return false;
}

int c_init_server()
{
	SERVER_CONTEXT * c = &g_server_context;
	int i;

	if (! create_msg_queue(&c->msg_queue, SERVER_MSG_QUEUE_SIZE))
		goto failed;

	INIT_LIST_HEAD(&c->clients_list);
	if (! start_listen())
	{
		printf("%s: start_listen failed\n", __func__);
		goto failed;
	}
	return true;
failed:
	c_deinit_server();
	return false;
}

static inline void select_server_fds()
{
	SERVER_CONTEXT * c = &g_server_context;
	struct list_head * pos;
	CLIENT * client;
	struct timeval t;
	int maxfd = 0;

	t.tv_sec = 1;
	t.tv_usec = 0;
	FD_ZERO(&c->r_fds);
	FD_ZERO(&c->w_fds);

	maxfd = MAX(maxfd, c->listen_sock);
	FD_SET(c->listen_sock, &c->r_fds);
	maxfd = MAX(maxfd, c->msg_queue.s_r);
	FD_SET(c->msg_queue.s_r, &c->r_fds);
	list_for_each(pos, &c->clients_list)
	{
		client = list_entry(pos, CLIENT, list);
		maxfd = MAX(maxfd, client->sock);
		switch(client->type) 
		{
		case UNKNOWN_CLIENT:
			FD_SET(client->sock, &c->r_fds);
			break;
		case HTTP_CLIENT:
			FD_SET(client->sock, &c->r_fds);
			if (client->http_period == SEND_HTTP_RESULT)
				FD_SET(client->sock, &c->w_fds);
			break;
		default:
			break;
		}
	}
	select(maxfd + 1, &c->r_fds, &c->w_fds, NULL, &t);
}

static inline bool parse_http_client_recved_data(CLIENT * client, int len)
{
	return c_parse_http_client_recved_data(client,len);
}

static inline bool parse_unknown_client_recved_data(CLIENT * client, int len)
{
	printf("%s:%d\n", __func__, 1);
	SERVER_CONTEXT * c = &g_server_context;
	if (client->len_of_temp_data_4_unknown_client)
	{
		memcpy(c->buffer, client->temp_buffer_4_unknown_client, client->len_of_temp_data_4_unknown_client);
		len += client->len_of_temp_data_4_unknown_client;
	}

	if (((len >= 4) && (0 == strncasecmp(c->buffer, "GET ", 4))) ||
		((len >= 5) && (0 == strncasecmp(c->buffer, "POST ", 5))))
	{
		client->tick = c->tick;
		client->type = HTTP_CLIENT;
		client->http_period = RECV_HTTP_HEADER;
		return parse_http_client_recved_data(client, len);
	}
	else if (len < 4)
	{
		memcpy(client->temp_buffer_4_unknown_client, c->buffer, len);
		client->len_of_temp_data_4_unknown_client = len;
		return true;
	}
	return false;
}

static inline bool parse_client_recved_data(CLIENT * client, int len)
{
	printf("Enter %s\n", __func__);
	SERVER_CONTEXT * c = &g_server_context;
	bool ret = false;

	switch(client->type) 
	{
	case UNKNOWN_CLIENT:
		ret = parse_unknown_client_recved_data(client, len);
		break;
	case HTTP_CLIENT:
		ret = parse_http_client_recved_data(client, len);
		break;
	default:
		break;
	}
	return ret;
}

static inline void close_http_client(CLIENT * client)
{
	SERVER_CONTEXT * c = &g_server_context;
	deinit_client(client);
	shutdown(client->sock, SHUT_WR);
	client->tick = c->tick;
	client->http_period = WAIT_HTTP_CLOSE;
}

static inline bool send_client_data(CLIENT * client)
{
	SERVER_CONTEXT * c = &g_server_context;
	int len;
	if (client->send_data_len)
	{
		len = send(client->sock, client->send_data + client->send_data_offset, client->send_data_len, MSG_NOSIGNAL);
		if (0 >= len)
		{
			printf("%s: send failed %d\n", __func__, 2);
			return false;
		}
		client->send_data_offset += len;
		client->send_data_len -= len;
	}

	if (! client->send_data_len)
	{
		client->send_data_offset = 0;
		if (client->type == HTTP_CLIENT)
		{
			if (client->http_period == SEND_HTTP_RESULT)
			{
				if (client->http_file)
				{
					if (feof(client->http_file))
					{
						close_http_client(client);
						//fclose(client->http_file);
						//client->http_file = NULL;
					}
					else
					{
						if (0 > (client->send_data_len = fread(client->send_data, 1, MIN_HTTP_SEND_BUFFER_SIZE, client->http_file)))
						{
							client->send_data_len = 0;
							return false;
						}
					}
				}
				else
				{
					close_http_client(client);
				}
			}
		}
	}
	printf("%s: exit\n", __func__);
	return true;
}

void print_buf(char *buf, int len)
{
	//return;

	int idx = 0;

	//print_info("buf len:%d\n", len);
	printf("buf len:%d\n", len);

	for (idx = 0; idx < len; idx++)
	{
		if (idx % 16 == 0)
		{
			//print_info("\n");
			printf("\n");
		}

		//print_info("0x%02X ", (unsigned char)buf[idx]);
		printf("0x%02X ", (unsigned char)buf[idx]);
	}

	//print_info("\n");
	printf("\n");
}

static inline void clients_handler()
{
	SERVER_CONTEXT * c = &g_server_context;
	struct list_head * pos, * next;
	CLIENT * client;
	int len;

	list_for_each_safe(pos, next, &c->clients_list)
	{
		client = list_entry(pos, CLIENT, list);
		if (FD_ISSET(client->sock, &c->r_fds))
		{
			FD_CLR(client->sock, &c->r_fds);
			
			if (client->type == HTTP_CLIENT)
			{
				if ((client->http_period == RECV_HTTP_HEADER) ||
					(client->http_period == RECV_HTTP_DATA))
					client->tick = c->tick;
			}
			
			if (client->type == UNKNOWN_CLIENT)
				len = recv(client->sock, c->buffer + client->len_of_temp_data_4_unknown_client, SERVER_BUFFER_SIZE - 1 - client->len_of_temp_data_4_unknown_client, 0);
			else
				len = recv(client->sock, c->buffer, SERVER_BUFFER_SIZE - 1, 0);

			//printf("%s",c->buffer);
			printf("recv %d",len);
			//print_buf(c->buffer,len);
			if (0 >= len)
			{
				printf("remove_client\n");
				remove_client(client);	
				continue;
			}
			if (! parse_client_recved_data(client, len))
			{
				remove_client(client);	
				continue;
			}
		}

		if (FD_ISSET(client->sock, &c->w_fds))
		{
			FD_CLR(client->sock, &c->w_fds);
			if (client->type == HTTP_CLIENT)
			{
				if (client->http_period == SEND_HTTP_RESULT)
					client->tick = c->tick;
			}
			if (! send_client_data(client))
			{
			}
		}
	}
}

static inline void add_client()
{
	SERVER_CONTEXT * c = &g_server_context;
	int sock;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	CLIENT * client;

	if (0 > (sock = accept(c->listen_sock,(struct sockaddr *)&addr,&addrlen)))
		return;

	if (NULL == (client = (CLIENT *)my_malloc(sizeof(CLIENT))))
	{
		close(sock);
		return;
	}
	memset(client, 0, sizeof(CLIENT));

	client->sock = sock;
	disable_socket_linger(client->sock);
	set_socket_timeout(client->sock, 20, 20);
	set_socket_nonblock(client->sock);
	set_socket_nodelay(client->sock);
	client->peer_ip = addr.sin_addr.s_addr;
	http_request_open(&client->hrc, HTTP_HOME, HTTP_DEFAULT_FILE);
	client->type = UNKNOWN_CLIENT;
	client->tick = c->tick;
	list_add(&client->list, &c->clients_list);
	c->clients_number ++;
	return;
}

static inline void new_client_handler()
{
	SERVER_CONTEXT * c = &g_server_context;
	if (FD_ISSET(c->listen_sock, &c->r_fds))
	{
		FD_CLR(c->listen_sock, &c->r_fds);
		add_client();
	}
}

int c_is_server_busy()
{
	SERVER_CONTEXT * c = &g_server_context;
	struct list_head * pos;
	CLIENT * client;
	list_for_each(pos, &c->clients_list)
	{
		client = list_entry(pos, CLIENT, list);
		if (client->busy) return true;
	}
	return false;
}

void msg_handler();

void c_do_server_handler()
{
	SERVER_CONTEXT * c = &g_server_context;
	unsigned long tick;
	select_server_fds();
	//c->tick = get_tickcount();
	clients_handler();
	msg_handler();
	new_client_handler();
}

static inline void stop_listen()
{
	SERVER_CONTEXT * c = &g_server_context;
	if (c->listen_sock)
	{
		printf("close sock success\n");
		close(c->listen_sock);
		c->listen_sock = 0;
	}
}

void c_deinit_server()
{
	SERVER_CONTEXT * c = &g_server_context;
	destroy_msg_queue(&c->msg_queue);
	stop_listen();
	clear_clients();
	memset(c, 0, sizeof(SERVER_CONTEXT));
}

static inline void deinit_client(CLIENT * client)
{
	SERVER_CONTEXT * c = &g_server_context;
	
	my_safe_free(client->send_data);
	my_safe_free(client->recv_data);
	if (client->http_file)
	{
		fclose(client->http_file);
		client->http_file = NULL;
	}
	http_request_close(&client->hrc);
}


static inline void handle_wm_search_msg(SEARCH_PARAM * param)
{
	/*printf("Enter %s\n", __func__);
	SERVER_CONTEXT * c = &g_server_context;
	unsigned char result = 1;
	int res,rows;
	char data[5][512]={0};
	FILE *fp_read;
	int flag_find=0;
	int i=0;
	int k=0;
	int totle_num=0;
	char search_word[80];
	char buf[24];
	char buf_abstract[150];
	char buf_sentence[1024];
    int sentence = 0;
	int ch;


	// json
	cJSON * fld;
	cJSON * root = cJSON_CreateArray();
	cJSON * root_ld = cJSON_CreateObject();
	char * json = NULL;

	strcpy(search_word,param->key);
	search_word[strlen(search_word)] = 0;

	cJSON_AddItemToObject(root_ld,"result",root);
	// get data
	
	fp_read = fopen("/work/tan/test/my_index/index.dex","r");
	if(fp_read == NULL)
	{
		printf("open index.dex failed\n");
		exit(1);
	}
	else
	{
		printf("open index.dex success\n");
		WORDINFO t[10];

		LOCATIONINFO *locate_p;

		 //while (!feof(fp_read))
		while( fread(&t[i],sizeof(WORDINFO),1,fp_read))
		{
			
			if(strcmp(t[i].word,search_word) == 0)
			{
				// printf("t[i].word=%s\n",t[i].word);
				flag_find = 1;
				locate_p = t[i].locate;

				while(locate_p)
				{
					FILEINFO *p_file;
					p_file = T__INDEX_CONTEXT->filelist;

					while(p_file && p_file->id !=locate_p->id)
					{
						p_file = p_file->next;
					}
					printf("file+ path + frequency=%s + %s +%d \n",p_file->filename,p_file->path,locate_p->frequency);
					
					
					//get file abstract ,first convert file into /tmp/convert_file.txt
					system("rm /tmp/convert_file.txt");
					char *convert_file_name = Process_File_Type(p_file->path);

					FILE *fp_abstract = fopen(convert_file_name,"rb");
					if(!fp_abstract)
					{
						printf("fopen /tmp/convert_file failed\n");
						exit(1);
					}
					
					else
					{
						while (!feof(fp_abstract) && (ch = fgetc(fp_abstract)) != EOF) 
						{
								totle_num++;
								if(!sentence)
								{
									if(isblankchar(ch))
										continue;
									else
									{
										i = 0;
										buf_sentence[i++] = ch;   
										sentence = 1;
										if (i == 1024)
											i--;     
									}                        
								}
								else if(sentence) 
								{
									if(isblankchar(ch))
									{ 
				                    
										buf_sentence[i++] = '\0';
										if (i == 1024)
											buf_sentence[--i] = '\0';
										sentence = 0;
				                        
										if(strlen(buf_sentence)>3)
										{ 
											if(strstr(buf_sentence,search_word))
											{
												printf("buf_sentence=%s\n",buf_sentence);
												totle_num=totle_num-strlen(buf_sentence);
												break;
											}
										}
									}
									else
									{
										buf_sentence[i++] = ch;    
										if (i == 1024)
											i--;
									}
								}
							}
						
				
						
						printf("totle_num=%d\n",totle_num);
						fseek(fp_abstract,totle_num-1,0);

						fread(buf_abstract,150,1,fp_abstract);
						del_line_symbol(&buf_abstract);
						strcpy(data[2],buf_abstract);

					}

					fclose(fp_abstract);
				
					strcpy(data[0],p_file->filename);
					strcpy(data[1],p_file->path);
					//strcpy(data[2],locate_p->frequency);
					
					cJSON_AddItemToArray(root,fld=cJSON_CreateObject());
					cJSON_AddStringToObject(fld,"title",data[0]);
					cJSON_AddStringToObject(fld,"path",data[1]);
					cJSON_AddStringToObject(fld,"abstract",data[2]);
					k++;
					locate_p = locate_p->next;

				}
				break;
			}    
			// i++;
			memset(&t[0],0,sizeof(WORDINFO));
		}
		if(flag_find == 0)
			printf("can't find this word \n");


	}
	
	cJSON_AddNumberToObject(root_ld, "rows", k);
	json=cJSON_Print(root_ld);
	// send to client
	//c_make_http_client_html_resp(param->client, c->buffer, content_len);
	*/
    char * json = "i am json data for rest api response";
    c_make_http_client_html_resp(param->client, json, strlen(json));
	//my_safe_free(param->key);
	//my_safe_free(param);
	//fclose(fp_read);
}


void msg_handler()
{
	SERVER_CONTEXT * c = &g_server_context;
	MSG msg;

	if (FD_ISSET(c->msg_queue.s_r, &c->r_fds))
	{
		FD_CLR(c->msg_queue.s_r, &c->r_fds);
		if (recv_msg(&c->msg_queue, &msg))
		{
			switch(msg.msg) 
			{
			case WM_SEARCH:
				handle_wm_search_msg((SEARCH_PARAM * )msg.param1);
				
				break;
			default:
				break;
			}
		}
	}
	//printf("server thread proc running\n");
	return;
}
