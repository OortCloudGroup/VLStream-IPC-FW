/*
 * @Author: wangxu
 * @LastEditTime: 2024-12-30 16:38:36
 */
#include <stdio.h>
// #include "../librtsp/include/rtp.h"
// #include "../librtsp/include/rtsp_parse.h"
// #include "../librtsp/include/rtsp_handler.h"
// #include "../librtsp/include/h264.h" //../librtsp/include/
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "pthread.h"
// #include "../librtsp/include/net.h"
#include "c_rtsp_server.h"
#include "../librtsp/rtsp_demo.h"
#include "c_queue_frame.h"

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
            rtsp_tx_video(session[0], frame.data, frame.size, ts);
        }

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
    signal(SIGINT, sig_proc);
    rtsp_del_session(session[0]);
	rtsp_del_demo(demo);
}
