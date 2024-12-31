/*
 * @Author: wangxu
 * @LastEditTime: 2024-12-30 16:38:36
 */

#include "server.h"

FrameQueue g_queue;

int main(int argc, char ** argv) 
{
	// Initialize queue
    init_queue(&g_queue,10);
	
	bool record_thread_alive = false;
	bool rtsp_thread_alive = false;
	bool server_thread_alive = false;

	if (! t_create_record_thread())
		goto quit;
	else
		record_thread_alive = true;

	if (! t_create_rtsp_thread())
		goto quit;
	else
		rtsp_thread_alive = true;

	if (! t_create_server_thread())
		goto quit;
	else
		server_thread_alive = true;
	sleep(1);

	while(1)
	{
		record_thread_alive = t_is_record_thread_alive();
		rtsp_thread_alive = t_is_rtsp_thread_alive();
		server_thread_alive = t_is_server_thread_alive();
		if(!record_thread_alive 
		|| !rtsp_thread_alive
		|| !server_thread_alive)
		{
			printf("main loop leave\n");
			break;
		}

		//usleep(500000);
		usleep(500000);
	}
	printf("Camera Quiting ...\n");
	return 0;
quit:
	printf("error Camera Quiting ...\n");
	return 0;
}
