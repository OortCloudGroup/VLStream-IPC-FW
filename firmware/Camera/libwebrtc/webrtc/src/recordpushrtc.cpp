#include "recordpushrtc.h"
#include <yangpush/YangPushCommon.h>
#include <yangutil/sys/YangLog.h>
#include <yangutil/sys/YangFile.h>
#include <yangutil/sys/YangSocket.h>
#include <yangutil/sys/YangIni.h>
#include <yangutil/sys/YangString.h>
#include <yangpush/YangPushFactory.h>

#include <iostream>
#include <chrono>
#include <thread>

RecordPushRtc::RecordPushRtc()
{
    
}

RecordPushRtc::~RecordPushRtc()
{
    closeAll();
}


void RecordPushRtc::initRecordPushRtc()
{
    m_context=new YangContext();
    m_context->init((char*)"yang_config.ini");
    m_context->avinfo.video.videoEncoderFormat=YangI420;
#if Yang_Enable_Openh264
    m_context->avinfo.enc.createMeta=0;
#else
    m_context->avinfo.enc.createMeta=1;
#endif
#if Yang_Enable_GPU_Encoding
    //using gpu encode
    m_context->avinfo.video.videoEncHwType=YangV_Hw_Nvdia;//YangV_Hw_Intel,  YangV_Hw_Nvdia,
    m_context->avinfo.video.videoEncoderFormat=YangI420;//YangI420;//YangArgb;
    m_context->avinfo.enc.createMeta=0;
#endif
    //视频参数初始化
    init();
    //设置日志等级以及日志文件
    yang_setLogLevel(m_context->avinfo.sys.logLevel);
    yang_setLogFile(m_context->avinfo.sys.enableLogFile);

    m_context->avinfo.sys.mediaServer=Yang_Server_Zlm;//Yang_Server_Srs/Yang_Server_Zlm/Yang_Server_Whip_Whep
    m_context->avinfo.rtc.rtcLocalPort=10000+yang_random()%15000;
    //视频类型
    m_videoType=Yang_VideoSrc_Camera;//Yang_VideoSrc_Camera/Yang_VideoSrc_Screen;

    m_isVr=0;


    char s[128]={0};
    memset(m_context->avinfo.rtc.localIp,0,sizeof(m_context->avinfo.rtc.localIp));
    yang_getLocalInfo(m_context->avinfo.sys.familyType,m_context->avinfo.rtc.localIp);
    sprintf(s,"http://%s:1985/rtc/v1/whip/?app=live&stream=livestream",m_context->avinfo.rtc.localIp);
    m_url = s;
    // ui->m_url->setText(s);
    
    m_isDrawmouse=true; //screen draw mouse
    m_screenInternal=33;
    m_hasAudio=m_videoType==Yang_VideoSrc_Screen?false:true;

    m_isStartRecord=false;

    m_initRecord=false;

    m_context->avinfo.rtc.enableDatachannel=yangfalse;

    //using h264 h265
    m_context->avinfo.video.videoEncoderType=Yang_VED_264;//Yang_VED_265;
    if(m_context->avinfo.video.videoEncoderType==Yang_VED_265){
        m_context->avinfo.enc.createMeta=1;
        m_context->avinfo.rtc.enableDatachannel=yangfalse;
    }
    m_context->avinfo.rtc.iceCandidateType=YangIceHost;

    //srs do not use audio fec
    m_context->avinfo.audio.enableAudioFec=yangfalse;
}

void RecordPushRtc::initMessage()
{
    m_message =  m_mf.createPushMessageHandle(m_hasAudio,false,m_videoType,&m_screenInfo,&m_outInfo,m_context,this,this);
    m_message->start();
}

void RecordPushRtc::startRecordPushWeb(const char* url)
{
    std::string TempUrl(url);
    std::cout << "ip:" << url << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    yang_post_message(YangM_Push_Connect_Whip,0,NULL,(void*)url);
}


void RecordPushRtc::initPreview()
{
    if(m_videoType==Yang_VideoSrc_Screen) {

        yang_post_message(YangM_Push_StartScreenCapture,0,NULL);
    }else if(m_videoType==Yang_VideoSrc_Camera){
        yang_post_message(YangM_Push_StartVideoCapture,0,NULL);
    }else if(m_videoType==Yang_VideoSrc_OutInterface){
        yang_post_message(YangM_Push_StartOutCapture,0,NULL);
    }
}

void RecordPushRtc::success()
{

}

void RecordPushRtc::failure(int32_t errcode)
{
    //错误代码
}

void RecordPushRtc::receiveSysMessage(YangSysMessage *mss, int32_t err)
{
    switch (mss->messageId) {
        case YangM_Push_Connect:
        {
            if(err){
                // ui->m_b_rec->setText("开始");
                m_isStartpush=!m_isStartpush;
                // ui->m_l_err->setText("push error("+QString::number(err)+")!");
                // QMessageBox::about(NULL,  "error","push error("+QString::number(err)+")!");
            }
        }
            break;
        case YangM_Push_Disconnect:
        {
            std::cout<<"push disconnect";
        }
            break;
        case YangM_Push_StartScreenCapture:
            if(m_context->avinfo.video.videoEncoderFormat==YangArgb)
                m_videoBuffer=NULL;
            else
                m_videoBuffer=m_pushfactory.getPreVideoBuffer(m_message);
            std::cout<<"message==="<<m_message<<"..prevideobuffer==="<<m_videoBuffer<<"....ret===="<<err;
            break;
        case YangM_Push_StartVideoCapture:
        {
            m_videoBuffer=m_pushfactory.getPreVideoBuffer(m_message);
            std::cout<<"message==="<<m_message<<"..prevideobuffer==="<<m_videoBuffer<<"....ret===="<<err;
            break;
        }
        case YangM_Push_StartOutCapture:
        {
            m_videoBuffer=m_pushfactory.getPreVideoBuffer(m_message);
            std::cout<<"message==="<<m_message<<"..prevideobuffer==="<<m_videoBuffer<<"....ret===="<<err;
            break;
        }
        case YangM_Push_SwitchToCamera:
            m_videoBuffer=m_pushfactory.getPreVideoBuffer(m_message);
            break;
        case YangM_Push_SwitchToScreen:
            m_videoBuffer=m_pushfactory.getPreVideoBuffer(m_message);
            break;
        }
}

const std::string &RecordPushRtc::getstrRtcUrl()
{
    return m_url;
}

void RecordPushRtc::init()
{
    m_context->avinfo.audio.enableMono=yangfalse;
    m_context->avinfo.audio.sample=48000;
    m_context->avinfo.audio.channel=2;
    m_context->avinfo.audio.enableAec=yangfalse;
    m_context->avinfo.audio.audioCacheNum=8;
    m_context->avinfo.audio.audioCacheSize=8;
    m_context->avinfo.audio.audioPlayCacheNum=8;

    m_context->avinfo.video.videoCacheNum=10;
    m_context->avinfo.video.evideoCacheNum=10;
    m_context->avinfo.video.videoPlayCacheNum=10;

    m_context->avinfo.audio.audioEncoderType=Yang_AED_OPUS;
    m_context->avinfo.rtc.rtcLocalPort=17000;
    m_context->avinfo.enc.enc_threads=4;

    memcpy(&m_screenInfo,&m_context->avinfo.video,sizeof(YangVideoInfo));
    // QDesktopWidget* desk=QApplication::desktop();
    // m_screenWidth=desk->screenGeometry().width();
    // m_screenHeight=desk->screenGeometry().height();
    
    m_screenWidth=1280;
    m_screenHeight=720;
    m_screenInfo.width=m_screenWidth;
    m_screenInfo.height=m_screenHeight;
    m_screenInfo.outWidth=m_screenWidth;
    m_screenInfo.outHeight=m_screenHeight;
}

void RecordPushRtc::initRecord()
{
    if(!m_initRecord){
        m_context->avinfo.audio.audioEncoderType=Yang_AED_AAC;
        m_context->avinfo.audio.sample=44100;
        m_context->avinfo.enc.createMeta=1;

        m_context->avinfo.video.videoCacheNum=200;
        m_context->avinfo.video.evideoCacheNum=200;
        m_context->avinfo.audio.audioCacheNum=500;
        m_context->avinfo.audio.audioCacheSize=500;
        m_initRecord=true;
    }
}

void RecordPushRtc::closeAll()
{
    if(m_context){
        // m_rt->stopAll();
        // m_rt=NULL;
        yang_delete(m_message);
        yang_delete(m_context);
    }
}
