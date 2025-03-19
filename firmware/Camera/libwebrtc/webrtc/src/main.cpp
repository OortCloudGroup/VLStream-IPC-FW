#include <iostream>

#include <yangpush/YangPushFactory.h>
//#include <video/yangrecordthread.h>
#include <yangutil/sys/YangSysMessageHandle.h>
#include <yangutil/sys/YangSocket.h>

#include "recordpushrtc.h"
#include <thread>
#include <chrono>

int main() {

    RecordPushRtc w;
    YangPushFactory mf;

    YangSysMessageHandle *sysmessage=mf.createPushMessageHandle(w.m_hasAudio,false,w.m_videoType,&w.m_screenInfo,&w.m_outInfo,w.m_context,&w,&w);

    w.m_message=sysmessage;
    sysmessage->start();

    const string m_url = w.getstrRtcUrl();// = "http://192.168.246.131:1985/rtc/v1/whip/?app=live&stream=livestream";
    //ui->m_c_whip->checkState()== Qt::CheckState::Checked?YangM_Push_Connect_Whip:YangM_Push_Connect
    // sleep::msleep(200);

    std::cout << "ip:" << m_url << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    yang_post_message(YangM_Push_Connect_Whip,0,NULL,(void*)m_url.c_str());

    
    while (1)
    {
        /* code */
    }
    
    return 0;
}
