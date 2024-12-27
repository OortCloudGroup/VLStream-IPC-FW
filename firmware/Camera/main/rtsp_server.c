/*
 * @Author: cijliu
 * @Date: 2021-02-04 16:04:16
 * @LastEditTime: 2021-02-26 17:01:59
 */
#include <stdio.h>
#include "../librtsp/include/rtp.h"
#include "../librtsp/include/rtsp_parse.h"
#include "../librtsp/include/rtsp_handler.h"
#include "../librtsp/include/h264.h" //../librtsp/include/
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "pthread.h"
#include "../librtsp/include/net.h"
#include "rtsp_server.h"
#include "rtsp_demo.h"
#include "queueFrame.h"

extern FrameQueue g_queue;

#define MAX_SESSION_NUM 64

//FILE *fp[MAX_SESSION_NUM][2] = {{NULL}};
rtsp_demo_handle demo;
rtsp_session_handle session[MAX_SESSION_NUM] = {NULL};
int session_count = 0;
// uint8_t *vbuf = NULL;
// uint8_t *abuf = NULL;

uint64_t ts = 0;
int vsize = 0, asize = 0;
int ret, ch;

static int flag_run = 1;
static void sig_proc(int signo)
{
	flag_run = 0;
}


ip_t * ipaddr;
tcp_t tcp;
int client;

int c_init_rtsp()
{
    demo = rtsp_new_demo(8554);
	if (NULL == demo) {
		printf("rtsp_new_demo failed\n");
		return -1;
	}


    // session[ch] = rtsp_new_session(demo, cfg.session_cfg[ch].path);
    // if (NULL == session[ch]) {
    //     printf("rtsp_new_session failed\n");
    //     continue;
    // }

    //rtsp_set_auth(session[ch], RTSP_AUTH_TYPE_BASIC, "admin", "123456");
    session[0] = rtsp_new_session(demo, "/live/video");
    if (NULL == session[0]) {
        printf("rtsp_new_session failed\n");
        return -1;
    }
    rtsp_set_video(session[0], RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
    rtsp_sync_video_ts(session[0], rtsp_get_reltime(), rtsp_get_ntptime());
    
    // if (fp[ch][1]) {
    //     rtsp_set_audio(session[ch], RTSP_CODEC_ID_AUDIO_G711A, NULL, 0);
    //     rtsp_sync_audio_ts(session[ch], rtsp_get_reltime(), rtsp_get_ntptime());
    // }

    // printf("==========> rtsp://127.0.0.1:8554%s for %s %s <===========\n", cfg.session_cfg[ch].path, 
    //     fp[ch][0] ? cfg.session_cfg[ch].video_file : "", 
    //     fp[ch][1] ? cfg.session_cfg[ch].audio_file : "");


	ts = rtsp_get_reltime();
	signal(SIGINT, sig_proc);
#ifdef __linux__
	signal(SIGPIPE, SIG_IGN);
#else
    
#endif

    return 0;
}

int c_do_rtsp_handler()
{
    if (flag_run) 
    {
	    uint8_t type = 0;

        Frame frame = dequeue(&g_queue);
        if(frame.data && frame.size > 0){
            //printf("当前时间: %ld",ts);
            rtsp_tx_video(session[0], frame.data, frame.size, ts);
        }
		// for (ch = 0; ch < session_count; ch++) {
		// 	if (fp[ch][0]) {
		// 	read_video_again:
		// 		ret = get_next_video_frame(fp[ch][0], &vbuf, &vsize);
		// 		if (ret < 0) {
		// 			fprintf(stderr, "get_next_video_frame failed\n");
		// 			flag_run = 0;
		// 			break;
		// 		}
		// 		if (ret == 0) {
		// 			fseek(fp[ch][0], 0, SEEK_SET);
		// 			if (fp[ch][1])
		// 				fseek(fp[ch][1], 0, SEEK_SET);
		// 			goto read_video_again;
		// 		}

		// 		if (session[ch])
		// 			rtsp_tx_video(session[ch], vbuf, vsize, ts);

		// 		type = 0;
		// 		if (vbuf[0] == 0 && vbuf[1] == 0 && vbuf[2] == 1) {
		// 			type = vbuf[3] & 0x1f;
		// 		}
		// 		if (vbuf[0] == 0 && vbuf[1] == 0 && vbuf[2] == 0 && vbuf[3] == 1) {
		// 			type = vbuf[4] & 0x1f;
		// 		}
		// 		if (type != 5 && type != 1)
		// 			goto read_video_again;
		// 	}

		// 	if (fp[ch][1]) {
		// 		ret = get_next_audio_frame(fp[ch][1], &abuf, &asize);
		// 		if (ret < 0) {
		// 			fprintf(stderr, "get_next_audio_frame failed\n");
		// 			break;
		// 		}
		// 		if (ret == 0) {
		// 			fseek(fp[ch][1], 0, SEEK_SET);
		// 			if (fp[ch][0])
		// 				fseek(fp[ch][0], 0, SEEK_SET);
		// 			continue;
		// 		}
		// 		if (session[ch])
		// 			rtsp_tx_audio(session[ch], abuf, asize, ts);
		// 	}
		// }

		do {
			ret = rtsp_do_event(demo);
			if (ret > 0)
				continue;
			if (ret < 0)
				return 1;
			usleep(20000);
		} while (rtsp_get_reltime() - ts < 1000000 / 25);
		if (ret < 0)
			return 1;
		ts += 1000000 / 25;
		printf(".");fflush(stdout);
	}else
    {
        return 0;
    }

    return 1;

}

void c_deinit_rtsp()
{
    //free(vbuf);
	//free(abuf);
	
	// for (ch = 0; ch < session_count; ch++) {
	// 	if (fp[ch][0])
	// 		fclose(fp[ch][0]);
	// 	if (fp[ch][1])
	// 		fclose(fp[ch][1]);
	// 	if (session[ch])
	// 		rtsp_del_session(session[ch]);
	// }
    signal(SIGINT, sig_proc);
    rtsp_del_session(session[0]);
	rtsp_del_demo(demo);
}

// void main()
// {
//     pthread_t rtsp_id;
//     pthread_create(&rtsp_id,NULL,rtsp_thread,&ip);
//     while(1){
//         sleep(1);
//     }

// }
