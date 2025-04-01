#include "../libpthread/libpthread.h"
#include "../libutility/libutility.h"

// #include "../libwebrtc/src/recordpushrtc.h"
#include "../libwebrtc/webrtc/src/recordpushrtc_capi.h"
#include "vls_webrtc_push.h"

#include "../include/x264.h"
#include "../include/x264_config.h"
// // #include "vls_record.h"
#include "c_h264_encoder.h"

extern FrameQueue g_queue;  // Reference global queue

#define WEBRTC_THREAD_ALIVE_THRESHOLD				(60 * 100)
#define WEBRTC_THREAD_PRIORITY						4

THREAD_CONTEXT g_webrtc_thread_context;
static void * webrtc_thread_function(void * arg);

bool vls_create_webrtc_thread()
{
	bool ret;
	g_webrtc_thread_context.alive_threshold = WEBRTC_THREAD_ALIVE_THRESHOLD;
	g_webrtc_thread_context.handler = webrtc_thread_function;
	g_webrtc_thread_context.priority = WEBRTC_THREAD_PRIORITY;
	if (! (ret = create_thread(&g_webrtc_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}

// 全局变量
x264_t *encoder = NULL;
x264_picture_t pic_in;
x264_picture_t pic_out;
x264_nal_t *nals;
int i_nal;
int current_width = 0;
int current_height = 0;

// 初始化 x264 编码器
int init_x264_encoder(int width, int height) {
    x264_param_t param;
    // 设置默认参数
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    // 设置视频分辨率
    param.i_width = width;
    param.i_height = height;
    // 设置帧率
    param.i_fps_num = 25;
    param.i_fps_den = 1;
    // 设置输出为 Annex - B 格式（包含起始码）
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    // 应用参数
    x264_param_apply_profile(&param, "baseline");

    // 打开编码器
    encoder = x264_encoder_open(&param);
    if (!encoder) {
        fprintf(stderr, "Failed to open x264 encoder.\n");
        return 0;
    }

    // 初始化输入图像
    x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);
    current_width = width;
    current_height = height;
    return 1;
}

// 释放 x264 编码器资源
void release_x264_encoder() {
    if (encoder) {
        x264_encoder_close(encoder);
        x264_picture_clean(&pic_in);
        encoder = NULL;
    }
}

// 定义 C 语言回调函数
void on_video_yuv_frame(const uint8_t* data, int w, int h, int len) 
{
	//printf("\nReceived frame: %dx%d, size=%d\n", w, h, len);

	if (!encoder || w != current_width || h != current_height) {
        if (encoder) {
            x264_encoder_close(encoder);
            x264_picture_clean(&pic_in);
        }
        if (!init_x264_encoder(w, h)) {
            return;
        }
    }

    // 填充 YUV 数据到 x264 图像结构
    int y_size = w * h;
    int uv_size = y_size / 4;
    // 复制 Y 分量
    memcpy(pic_in.img.plane[0], data, y_size);
    // 复制 U 分量
    memcpy(pic_in.img.plane[1], data + y_size, uv_size);
    // 复制 V 分量
    memcpy(pic_in.img.plane[2], data + y_size + uv_size, uv_size);

    // 编码一帧
    int frame_size = x264_encoder_encode(encoder, &nals, &i_nal, &pic_in, &pic_out);
    if (frame_size > 0) {
        // 计算一帧完整数据的总大小
        int total_size = 0;
        for (int i = 0; i < i_nal; ++i) {
            total_size += nals[i].i_payload;
        }

        // 分配内存来存储一帧完整的数据
        uint8_t *frame_data = (uint8_t *)malloc(total_size);
        if (!frame_data) {
            fprintf(stderr, "Memory allocation failed.\n");
            return;
        }

        // 合并所有 NAL 单元到一帧数据中
        int offset = 0;
        for (int i = 0; i < i_nal; ++i) {
            memcpy(frame_data + offset, nals[i].p_payload, nals[i].i_payload);
            offset += nals[i].i_payload;
        }

        // 发送一帧完整的数据
        enqueue(&g_queue,frame_data, total_size);

        // 释放内存
        free(frame_data);
    }
}

static void * webrtc_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;

	// if (c_init_wertc() != 0)
	// {
	// 	printf("%s: reboot\n", __func__);
	// 	//c_reboot();
	// 	return NULL;
	// }
	  // 创建对象
	void* handle = record_push_create();
	 //初始化参数
	record_push_init(handle);
	//消息队列初始化
	record_message_init(handle);

	 // 设置回调
	set_video_yuv_callback(handle, on_video_yuv_frame);
	while (1)
	{
		
		start_push_camera(handle,"http://192.168.246.131:1985/rtc/v1/whip/?app=live&stream=livestream");
		while (1)
		{
			/* code */
		}
		
	}
	release_x264_encoder();
	// 销毁对象
    record_push_destroy(handle);
	printf("wertc quited\n");
	c->quited = true;
	
	return NULL;
}

