
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

#define LINE_SIZE				1024

typedef struct tagHTTPFile
{
	bool https;
	FILE * fp;
	char * buffer;
} HTTP_STREAM;
#define MAX_HTTP_LINE_SIZE						10240
#define MAX_HTTP_HEADER_SIZE					(10240 + 4096)
static inline void unescape(char *s)
{
	unsigned int c;
	
	while ((s = strpbrk(s, "%+"))) 
	{
		/* Parse %xx */
		if (*s == '%') 
		{
			sscanf(s + 1, "%02x", &c);
			*s++ = (char) c;
			strncpy(s, s + 2, strlen(s) + 1);
		}
		/* Space is special */
		else if (*s == '+')
			*s++ = ' ';
	}
}

void http_request_close(HTTP_REQUEST_CONTEXT * c)
{
	struct list_head * pos, * next;
	HTTP_PARAM * param;

	if (c->absolute_path)
		my_free(c->absolute_path);
	if (c->user)
		my_free(c->user);
	if (c->jsonp_callback)
		my_free(c->jsonp_callback);
	if (c->next_url)
		my_free(c->next_url);
	list_for_each_safe(pos, next, &c->params_list)
	{
		param = list_entry(pos, HTTP_PARAM, list);
		list_del(pos);
		if (param->name)
			my_free(param->name);
		my_free(param);
	}

	memset(c, 0, sizeof(HTTP_REQUEST_CONTEXT));
	INIT_LIST_HEAD(&c->params_list);
	
	return;
}

int http_request_parse(HTTP_REQUEST_CONTEXT * c, char * data, unsigned int len)
{
	char * line, * end, * path, * params, * protocol, * next, * temp;
	HTTP_PARAM * param;
	int ret = HTTP_CONTINUE;

	if (c->data)
	{
		if (NULL == (line = (char *)my_malloc(c->len + len + 1)))
			return HTTP_INTERNAL_ERROR;
		memcpy(line, c->data, c->len);
		memcpy(line + c->len, data, len);
		my_free(c->data);
		c->data = line;
		c->len += len;
		c->total += len;
		len = c->len;
	}
	else
	{
		line = data;
	}
	line[len] = 0;

	while (1)
	{
		if (NULL == (end = strstr(line, "\r\n")))
		{
			if (len >= MAX_HTTP_LINE_SIZE)
				return HTTP_BAD_REQUEST;
			else if (c->total >= MAX_HTTP_HEADER_SIZE)
				return HTTP_BAD_REQUEST;
			
			ret = HTTP_CONTINUE;
			break;
		}
		* end = 0;
		//printf("%s1: %s %d\n", __func__, line, len);
		if (! c->first_line_ok)
		{
			temp = line;
			if (0 == strncasecmp(line, "GET ", 4))
				c->method = GET;
			else if (0 == strncasecmp(line, "POST ", 5))
				c->method = POST;
			else
				return HTTP_BAD_REQUEST;

			path = line;
			strsep(&path, " ");
			while (*path == ' ') path++;
			protocol = path;
			strsep(&protocol, " ");
			params = path;
			strsep(&params, "?");
			
			if (path[0] != '/') 
				return HTTP_BAD_REQUEST;		//	only support absolute path
			if (path[1] == 0)
				c->absolute_path = str_combine(c->home, c->default_file); // set the default_file when get / root for client.
			else
				c->absolute_path = str_combine(c->home, path);
			if (! c->absolute_path)
				return HTTP_INTERNAL_ERROR;
			c->relative_path = c->absolute_path + strlen(c->home);
			
			if (params)
			{
				next = params;
				while ((params = (char *)strsep(&next, "&;")))
				{
					if (NULL == (param = (HTTP_PARAM *)my_malloc(sizeof(HTTP_PARAM))))
						return HTTP_INTERNAL_ERROR;
					if (NULL == (param->name = str_assign(params)))
					{
						my_free(param);
						return HTTP_INTERNAL_ERROR;
					}
					param->value = param->name;
					strsep(&param->value,"=");
					if (! param->value)
					{
						my_free(param->name);
						my_free(param);
					}
					else
					{
						unescape(param->value);
						list_add(&param->list, &c->params_list);
					}
				}
	//			if (ret)
	//				break;
			}
			c->first_line_ok = true;
			line = temp;
		}
		else if (line[0] == 0)
		{
			len -= 2;
			line += 2;
			ret = HTTP_OK;
			//printf("%s3: %d\n", __func__, len);
			break;
		}
		else if (strncasecmp(line,"Authorization:",14) == 0)
		{
			temp = line;
			line += 14;
			line += strspn(line," \t");
			if (strncasecmp(line,"Digest ",7))
				return HTTP_BAD_REQUEST;
			line = temp;
			c->realm_str = NULL;

			if (NULL == (c->realm_str = (char *)my_malloc(strlen(line) + 1))) 
				return HTTP_INTERNAL_ERROR;
			memcpy(c->realm_str,line,strlen(line));
			c->realm_str[strlen(line)] = 0;
		}
		else if (strncasecmp(line,"Content-Length:",15) == 0) 
		{
			temp = line;
			line += 15;
			line += strspn(line," \t");
			c->content_len = strtoul(line,NULL,0);
			line = temp;
		}
		else if (strncasecmp(line,"Content-Type:",13) == 0) 
		{
			temp = line;
			line = strstr(line,"boundary=");
			if (line)
			{
				c->boundary_len = strlen(line + 9);
				memcpy(c->boundary,line + 9,c->boundary_len);
				c->boundary[c->boundary_len] = 0;
			}
			line = temp;
		}
		
		end += 2;
		len -= end - line;
		line = end;
		//printf("%s2: %d\n", __func__, len);
	}
	
	if (line != c->data)
	{
		if (c->data)
		{
			my_free(c->data);
			c->data = NULL;
			c->len = 0;
		}
		if (len)
		{
			if (NULL == (c->data = (char *)my_malloc(len)))
				return HTTP_INTERNAL_ERROR;
			memcpy(c->data, line, len);
			c->len = len;
		}
	}
	
	return ret;	
}

// 根据参数名称，在参数列表中提取参数值
char * http_request_get_param(HTTP_REQUEST_CONTEXT * c, const char * name)
{
	struct list_head * pos;
	HTTP_PARAM * param;
	if (! name)
		return NULL;
	list_for_each(pos, &c->params_list)
	{
		param = list_entry(pos, HTTP_PARAM, list);
		if ((param->name) && (0 == (strcasecmp(name, param->name))))
			return param->value;
	}
	return NULL;
}
