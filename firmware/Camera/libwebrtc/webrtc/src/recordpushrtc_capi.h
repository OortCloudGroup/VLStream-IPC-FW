#ifndef RECORDPUSHRTC_INIT_H
#define RECORDPUSHRTC_INIT_H
// recordpush_capi.h
#ifdef __cplusplus
extern "C" {
#endif


// 创建对象
void* record_push_create();

// 初始化配置
void record_push_init(void* handle);

//初始化消息队列
void record_message_init(void* handle);

//开始推流
void start_push_camera(void* handle , const char* url);

// 销毁对象
void record_push_destroy(void* handle);

#ifdef __cplusplus
}
#endif

#endif