//
// Copyright (c) 2019-2022 yanggaofeng
//
#ifndef YANGTHREAD_H
#define YANGTHREAD_H


#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

#include "yangplayer/YangWinPlayFactroy.h"
#include "yangutil/buffer/YangVideoBuffer.h"


// 假设以下为用户自定义类型，保留原有接口
// class YangContext;
// class YangPlayWidget;
// class YangVideoBuffer;
// struct YangColor;
// struct YangFrame;

class YangRecordThread {
public:
    YangRecordThread();
    ~YangRecordThread();

        // 定义视频帧回调类型（参数：数据指针、宽、高、数据长度）
    using VideoFrameCallback = std::function<void(const uint8_t*, int, int, int)>;

    void initPara(YangContext* par);  // 初始化参数
    void stopAll();                   // 停止线程
    void start();                    // 启动线程

    // 原有成员变量（根据实际类型调整）
    std::atomic<int32_t> m_isLoop;
    // YangPlayWidget* m_video;
    YangVideoBuffer* m_videoBuffer;
    int32_t m_sid;
    int32_t showType;
    YangColor m_bgColor;
    YangColor m_textColor;
    int32_t m_videoPlayNum;
    YangContext* m_para;
    YangFrame m_frame;

     // 设置回调函数（线程安全）
    void setVideoCallback(const VideoFrameCallback& callback);
private:
    void run();                      // 线程执行体
    void render();                   // 渲染逻辑
    //void closeAll();                 // 关闭资源

    std::thread m_thread;            // 替换 QThread 为 std::thread
    std::atomic<bool> m_running;     // 线程运行标志
    std::atomic<bool> m_stopFlag;    // 停止标志

        // 新增成员
    VideoFrameCallback m_videoCallback;  // 回调函数
    std::mutex m_callbackMutex;          // 保护回调的互斥锁
};

// class YangRecordThread : public thread
// {
// public:
//     YangRecordThread();
//     virtual ~YangRecordThread();
//     int32_t m_isLoop;
//     void initPara(YangContext *par);
//     // YangPlayWidget *m_video;
//     YangVideoBuffer *m_videoBuffer;
//     int32_t m_sid;
//     void stopAll();
//     int32_t showType;

// private:
//     void render();
//     void closeAll();

//     //void initPlaySdl(YangRecordThreadWin *pwin);
//     int32_t m_isStart;
//     YangColor m_bgColor;
//     YangColor m_textColor;
//     int32_t m_videoPlayNum;
//     YangContext *m_para;
//     YangFrame m_frame;

// private:
//     virtual void run();
// };

#endif // YANGTHREAD_H
