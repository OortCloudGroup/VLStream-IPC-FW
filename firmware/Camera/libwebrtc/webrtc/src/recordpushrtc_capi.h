#ifndef RECORDPUSHRTC_INIT_H
#define RECORDPUSHRTC_INIT_H
// recordpush_capi.h
#ifdef __cplusplus
extern "C" {
#endif

// 定义视频帧回调函数类型 (C语言兼容)
typedef void (*VideoFrameCallback)(const uint8_t* data, int32_t width, int32_t height, int32_t length);


// 创建对象
void* record_push_create();

// 初始化配置
void record_push_init(void* handle);

//初始化消息队列
void record_message_init(void* handle);

//开始推流
void start_push_camera(void* handle , const char* url);

//设置视频帧回调
void set_video_yuv_callback(void* handle, VideoFrameCallback callback);

// 销毁对象
void record_push_destroy(void* handle);

#ifdef __cplusplus
}
#endif

#endif