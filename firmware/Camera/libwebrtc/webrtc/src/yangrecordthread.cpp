//
// Copyright (c) 2019-2022 yanggaofeng
//
#include "yangrecordthread.h"
#include <iostream>

YangRecordThread::YangRecordThread() 
    : m_isLoop(0),
    //   m_video(nullptr),
      m_videoBuffer(nullptr),
      m_sid(0),
      showType(0),
      m_videoPlayNum(0),
      m_para(nullptr),
      m_running(false),
      m_stopFlag(false) 
{
    m_bgColor = {0, 0, 0};
    m_textColor = {0, 0, 255};
}

YangRecordThread::~YangRecordThread() {
    stopAll();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    //closeAll();
}

void YangRecordThread::initPara(YangContext* par) {
    m_para = par;
    if (m_para) {
        m_videoPlayNum = m_para->avinfo.video.videoPlayCacheNum;
    }
}

void YangRecordThread::start() {
    if (m_running) return;
    m_running = true;
    m_stopFlag = false;
    m_thread = std::thread(&YangRecordThread::run, this);
}

void YangRecordThread::stopAll() {
    m_stopFlag = true;
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void YangRecordThread::setVideoCallback(const VideoFrameCallback &callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_videoCallback = callback;
}

void YangRecordThread::run()
{
    m_isLoop = 1;
    while (m_running && m_isLoop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        render();
    }
    m_isLoop = 0;
}

void YangRecordThread::render() {
    if (m_videoBuffer && m_videoBuffer->size() > 0) {
        uint8_t* t_vb = m_videoBuffer->getVideoRef(&m_frame);
        if (t_vb /*&& m_video*/ && m_videoBuffer->m_width > 0) {
            //m_video->PlayOneFrame(t_vb, m_videoBuffer->m_width, m_videoBuffer->m_height);
             // 保存为H264帧
            // std::cout << "huamian1111111\n" << m_videoBuffer->m_width << m_videoBuffer->m_height << "length" <<m_videoBuffer->m_length;
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            if (m_videoCallback) {
                m_videoCallback(t_vb, 
                              m_videoBuffer->m_width, 
                              m_videoBuffer->m_height, 
                              m_videoBuffer->m_length);
            }
        }
        t_vb = nullptr;
    }
}


