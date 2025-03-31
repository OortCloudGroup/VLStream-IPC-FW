
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

#include <netinet/ip.h>      // for icmp_ping
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include "c_server.h"

bool create_msg_queue(MSG_CONTEXT * c, unsigned int size)
{
	int ls;
	struct sockaddr_in host;
	int len = sizeof(host);

	memset(c, 0, sizeof(MSG_CONTEXT));

	if (0 > (ls = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("%s: can not create socket %d\n", __func__, ls);
		return false;
	}
	set_socket_reuseaddr(ls);
	host.sin_family = AF_INET;
	host.sin_addr.s_addr = inet_addr("127.0.0.1");
	host.sin_port = 0;
	if (0 > bind(ls,(struct sockaddr *)&host,sizeof(host)))
	{
		printf("%s: can not bind socket\n", __func__);
		close(ls);
		return false;
	}
	if (0 > getsockname(ls,(struct sockaddr *)&host,&len))
	{
		printf("%s: getsockname error\n", __func__);
		close(ls);
		return false;
	}
	listen(ls, 1);

	if (0 > (c->s_r = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("%s: can not create socket %d\n", __func__, c->s_r);
		close(ls);
		return false;
	}
	set_socket_nonblock(c->s_r);
	connect(c->s_r, (const struct sockaddr *)&host, len);
	if (0 > (c->s_w = accept(ls, (struct sockaddr *)&host,&len)))
	{
		printf("%s: accept error %d\n", __func__, c->s_w);
		close(ls);
		close(c->s_r);
		return false;
	}
	close(ls);
	set_socket_buff_size(c->s_r, 1024, size);
	set_socket_buff_size(c->s_w, size, 1024);
	pthread_mutex_init(&c->mutex, NULL);
	return true;
}


bool send_msg(MSG_CONTEXT * c, MSG * msg)
{
	msg->magic = MSG_MAGIC;		
	pthread_mutex_lock(&c->mutex);
	if (! socket_send(c->s_w, (const char *)msg, sizeof(MSG)))
	{
		pthread_mutex_unlock(&c->mutex);
		return false;
	}
	pthread_mutex_unlock(&c->mutex);
	return true;
}


bool recv_msg(MSG_CONTEXT * c, MSG * msg)
{
	int ret, offset;
	unsigned int magic = MSG_MAGIC;

	if (0 > (ret = recv(c->s_r, c->buffer + c->len, sizeof(MSG) - c->len, 0)))
		return false;

	c->len += ret;
	if (c->len != sizeof(MSG))
		return false;

	for (offset = 0; offset < sizeof(MSG) - 4; offset ++)
	{
		if (0 == memcmp(c->buffer + offset, &magic, 4))
			break;
	}
	if (offset)
	{
		c->len -= offset;
		memcpy(c->buffer, c->buffer + offset, c->len);
		return false;
	}

	memcpy((char *)msg, c->buffer, sizeof(MSG));
	c->len = 0;
	return true;
}

void destroy_msg_queue(MSG_CONTEXT * c)
{
	pthread_mutex_destroy(&c->mutex);
	close(c->s_r);
	close(c->s_w);
	memset(c, 0, sizeof(MSG_CONTEXT));
}

bool is_socket_writeable(int s, unsigned int ms)
{
	fd_set fds;
	struct timeval t;
	t.tv_sec = ms / 1000;
	t.tv_usec = ms % 1000;
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	if (1 != select(s + 1, NULL, &fds, NULL, &t))
		return false;
	
	return true;
}

bool is_socket_readable(int s, unsigned int ms)
{
	fd_set fds;
	struct timeval t;
	t.tv_sec = ms / 1000;
	t.tv_usec = ms % 1000;
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	if (1 != select(s + 1, &fds, NULL, NULL, &t))
		return false;
	
	return true;
}

bool socket_connect(int s, const char * host, unsigned short port, unsigned int timeout)
{
	struct sockaddr_in addr;
	//struct hostent * lphost;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (INADDR_NONE == (addr.sin_addr.s_addr = my_gethostbyname(host)))
		return false;
	/*if (INADDR_NONE == (addr.sin_addr.s_addr = inet_addr(host)))
	{
		lphost = gethostbyname(host);
		if (lphost != NULL)
		{
			addr.sin_addr.s_addr = ((struct in_addr *)(lphost->h_addr))->s_addr;
		}
		else
		{
			printf("%s: can not resolve ip of %s\n", __func__, host);
			return false;
		}
	}*/
	
	if (0 > connect(s, (struct sockaddr *)&addr, sizeof(addr))) 
	{
		printf("%s: can not connect to %s:%d\n", __func__, host, port);
		return false;
	}
	
	return true;
}

bool socket_send(int s, const char * data, unsigned int len)
{
	unsigned int offset = 0;
	int ret;
	
	while (len)
	{
		if (0 > (ret = send(s, data + offset, len, 0)))
		{
			printf("%s: error %d\n", __func__, ret);
			return false;
		}
		offset += ret;
		len -= ret;
	}
	
	return true;
}

bool socket_recv(int s, char * data, unsigned int len)
{
	unsigned int offset = 0;
	int ret;
	
	while (len)
	{
		if (0 > (ret = recv(s, data + offset, len, 0)))
		{
			printf("%s: error %d\n", __func__, ret);
			return false;
		}
		offset += ret;
		len -= ret;
	}
	
	return true;
}

unsigned long my_gethostbyname(const char * server)
{
	unsigned long ip;
	struct hostent host, * lphost;
	char * dnsbuffer;
	int rc;

	if (INADDR_NONE != (ip = inet_addr(server)))
		return ip;
	if (NULL == (dnsbuffer = (char *)my_malloc(8192)))
		return INADDR_NONE;
	if (gethostbyname_r(server, &host, dnsbuffer, 8192, &lphost, &rc) || (! lphost))
	{
		res_init();
		if (gethostbyname_r(server, &host, dnsbuffer, 8192, &lphost, &rc) || (! lphost))
		{
			printf("can not resolve ip of %s\n", server);
			ip = INADDR_NONE;
		}
		else
		{
			ip = ((struct in_addr *)(lphost->h_addr))->s_addr;
		}
	}
	else
	{
		ip = ((struct in_addr *)(lphost->h_addr))->s_addr;
	}
	
	my_free(dnsbuffer);
	return ip;
}

//*
char * get_local_ip_addr(const char * if_name)
{
    int i=0;
    int sockfd;
    struct ifconf ifconf;
    unsigned char buf[512];
    struct ifreq *ifreq;
    //初始化ifconf
    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("socket" );
        return NULL;
    }
    ioctl(sockfd, SIOCGIFCONF, &ifconf); //获取所有接口信息
    //接下来一个一个的获取IP地址
    close(sockfd);
    ifreq = (struct ifreq*)buf;
    for (i=(ifconf.ifc_len/sizeof (struct ifreq)); i>0; i--)
    {
        // if(ifreq->ifr_flags == AF_INET){ //for ipv4
        printf("%s\n",ifreq->ifr_name);
        printf("%s\n",inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
        if (strcmp(ifreq->ifr_name, if_name) == 0) 
            return inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
        ifreq++;
        // }
    }
    return NULL;
}

// return 
//. -1 error
//. 0 no connect
//. 1 connect
int dns_client(const char * server)
{
	time_t ident;
	int fd;
	int rc;
	int serveraddrlent;
	char *q;
	unsigned char *p;
	unsigned char *countp;
	unsigned char reqBuf[512] = {0};
	unsigned char rplBuf[512] = {0};
	struct sockaddr_in serveraddr;

	//udp
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1)
	{
		perror("error create udp socket");
		return -1;
	}

	time(&ident);
	//copy
	p = reqBuf;
	//Transaction ID
	*(p++) = ident;
	*(p++) = ident>>8;
	//Header section
	//flag word = 0x0100
	*(p++) = 0x01;
	*(p++) = 0x00;
	//Questions = 0x0001
	//just one query
	*(p++) = 0x00;
	*(p++) = 0x01;
	//Answer RRs = 0x0000
	//no answers in this message
	*(p++) = 0x00;
	*(p++) = 0x00;
	//Authority RRs = 0x0000
	*(p++) = 0x00;
	*(p++) = 0x00;
	//Additional RRs = 0x0000
	*(p++) = 0x00;
	*(p++) = 0x00;
	//Query section
	countp = p;  
	*(p++) = 0;
	for(q=(char*)server; *q!=0; q++)
	{
		if(*q != '.')
		{
			(*countp)++;
			*(p++) = *q;
		}
		else if(*countp != 0)
		{
			countp = p;
			*(p++) = 0;
		}
	}
	if(*countp != 0)
		*(p++) = 0;

	//Type=1(A):host address
	*(p++)=0;
	*(p++)=1;
	//Class=1(IN):internet
	*(p++)=0;
	*(p++)=1;

	printf("\nRequest:\n");
	//printmessage(reqBuf);

	//fill
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(53);
	char * ip = get_local_ip_addr("br-lan");
	//printf("%s\n",ip);
	if (ip)
		serveraddr.sin_addr.s_addr = inet_addr("8.8.8.8");
	else
		serveraddr.sin_addr.s_addr = inet_addr(ip);
	
	//send to DNS Serv
	if(sendto(fd,reqBuf,p-reqBuf,0,(void *)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("error sending request");
		close(fd);
		return -1;
	}

	// wait for reply add in 2015.2.12
	printf("wait for reply %s\n", __func__);
	fd_set set;   
	struct timeval tv = {10,0};   
	int i = 0;   
	FD_ZERO(&set);   
	FD_SET(fd, &set);   
	//if ((i = select(fd + 1, &set, NULL, NULL, &tv)) > 0)
	select(fd + 1, &set, NULL, NULL, &tv); 
	//if ((i = select(cwc->fd + 1, &set, NULL, NULL, NULL)) > 0)
	{ 
		printf("receive data\n");   
		if (FD_ISSET(fd, &set)) 
		{
			close(fd);
			return 1;
			goto recev_reply; //len = read(fd, buf, MAXLINE);
		}
		else
		{
			close(fd);
			return 0;
			goto exit_label;
		}
	} 
// no use of blew
recev_reply:    
	//recev the reply
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddrlent = sizeof(serveraddr);
	rc = recvfrom(fd,&rplBuf,sizeof(rplBuf),0,(void *)&serveraddr,&serveraddrlent);
	if(rc < 0)
	{
		perror("error receiving request\n");
		return -1;
	}  

	//print out results
	printf("\nReply:\n");
	//printmessage(rplBuf);

exit_label:
	//exit
	printf("Program Exit\n");
	return 0; 
}
//*/
