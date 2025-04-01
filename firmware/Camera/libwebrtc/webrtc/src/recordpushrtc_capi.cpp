#include "recordpushrtc.h"
#include "recordpushrtc_capi.h"

#include <yangpush/YangPushFactory.h>
//#include <video/yangrecordthread.h>
#include <yangutil/sys/YangSysMessageHandle.h>
#include <yangutil/sys/YangSocket.h>

#ifdef __cplusplus
extern "C" {
#endif


void* record_push_create() {
    return new RecordPushRtc();
}

void record_push_init(void* handle) 
{
    RecordPushRtc* obj = static_cast<RecordPushRtc*>(handle);
    obj->initRecordPushRtc();
}

void record_message_init(void* handle) {
    RecordPushRtc* obj = static_cast<RecordPushRtc*>(handle);
    obj->initMessage();
}

void start_push_camera(void* handle , const char* url)
{
    RecordPushRtc* obj = static_cast<RecordPushRtc*>(handle);
    obj->startRecordPushWeb(url);
}

void set_video_yuv_callback(void* handle, VideoFrameCallback callback)
{
    RecordPushRtc* obj = static_cast<RecordPushRtc*>(handle);
    obj->setVideoCallback(callback);
}

void record_push_destroy(void* handle) {
    delete static_cast<RecordPushRtc*>(handle);
}
#ifdef __cplusplus
}
#endif

