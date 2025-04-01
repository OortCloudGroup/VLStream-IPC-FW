/*

 * @Author: wangxu
 * @LastEditTime: 2024-12-30 16:38:36
 */

#include "server.h"

FrameQueue g_queue;

int main(int argc, char ** argv) 
{
	// Initialize queue
    init_queue(&g_queue,100);
	
	// bool vls_record_thread_alive = false;
	bool vls_rtsp_thread_alive = false;
	bool vls_server_thread_alive = false;
	//bool vls_uplaod_thread_alive = false;
	bool vls_webrtc_thread_alive = false;

	// if (! vls_create_record_thread())
	// 	goto quit;
	// else
	// 	vls_record_thread_alive = true;

	if (! vls_create_rtsp_thread())
		goto quit;
	else
		vls_rtsp_thread_alive = true;

	if (! vls_create_server_thread())
		goto quit;
	else
		vls_server_thread_alive = true;

	if(! vls_create_upload_thread())
		goto quit;
	// else
	// 	vls_uplaod_thread_alive = true;

	if(! vls_create_webrtc_thread())
		goto quit;
	else
		vls_webrtc_thread_alive = true;
	

	sleep(1);

	while(1)
	{
		// vls_record_thread_alive = vls_is_record_thread_alive();
		vls_rtsp_thread_alive = vls_is_rtsp_thread_alive();
		vls_server_thread_alive = vls_is_server_thread_alive();
		//vls_uplaod_thread_alive = vls_is_upload_thread_alive();
		
		//!vls_record_thread_alive 
		if(!vls_rtsp_thread_alive
		|| !vls_server_thread_alive)  //|| !vls_uplaod_thread_alive
		{
			printf("main loop leave\n");
			break;
		}

		usleep(500000);
	}
	printf("Camera Quiting ...\n");
	return 0;
quit:
	printf("error Camera Quiting ...\n");
	return -1;

}
