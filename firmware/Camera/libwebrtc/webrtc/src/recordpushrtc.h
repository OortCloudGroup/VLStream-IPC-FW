//
// Copyright (c) 2019-2022 yanggaofeng
//
#ifndef RECORDPUSHRTC_H
#define RECORDPUSHRTC_H

#include <yangutil/yangavinfotype.h>
//#include <yangpush/YangPushHandle.h>
#include <yangutil/sys/YangSysMessageI.h>
//#include <yangutil/sys/YangWindowsMouse.h>
#include <yangutil/sys/YangSysMessageHandle.h>
#include <yangpush/YangPushFactory.h>

#include "yangrecordthread.h"
#include "recordpushrtc_capi.h"

#define Yang_SendVideo_ 0

class RecordPushRtc : public YangSysMessageI,public YangSysMessageHandleI
{

public:
    RecordPushRtc();
    ~RecordPushRtc();
    //初始化RTC
    void initRecordPushRtc();
    //初始化消息
    void initMessage();
    
    void startRecordPushWeb(const char* url);
 
   // YangPushHandle *m_rec;
    YangContext *m_context;
    // YangRecordThread* m_rt;
//    YangPlayWidget *m_win0;
//    QHBoxLayout *m_hb0;
    int m_videoType;
    bool m_hasAudio;

    YangVideoInfo m_screenInfo;
    YangVideoInfo m_outInfo;

    YangSysMessageHandle *m_message;
    //视频帧回调函数
    void setVideoCallback(VideoFrameCallback callback);
    // 返回视频帧函数据
    void onVideoFrame(const uint8_t* data, int32_t width, int32_t height, int32_t length);
public:
    void initPreview();
    void success();
    void failure(int32_t errcode);
    void receiveSysMessage(YangSysMessage *mss, int32_t err);
    // void initVideoThread(YangRecordThread *prt);
    const std::string& getstrRtcUrl();
    
    void closeAll();
private:
    void init();
    void initRecord();
    bool m_initRecord;


private:
    int m_screenWidth;
    int m_screenHeight;
    int32_t m_isStartpush;
    bool m_isStartRecord;
    int32_t m_isVr;
    bool m_isDrawmouse;
    string m_url;
    int32_t m_screenInternal;
    YangPushFactory m_pushfactory;

    // YangVideoBuffer *m_videoBuffer;

    YangPushFactory m_mf;
    //摄像头画面线程
    YangRecordThread m_rt;
    //回调
    VideoFrameCallback m_videoCallback = nullptr;
};

#endif // RECORDPUSHRTC_H
