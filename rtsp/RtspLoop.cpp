//
// Created by fuck on 18-8-24.
//

#include "RtspLoop.h"

RtspLoop* RtspLoop::_instance = nullptr;

static void* RtspLoop_fuction(void* p)
{
    RtspLoop* ptr = static_cast<RtspLoop*>(p);
    ptr->loop();
}

bool RtspLoop::loop() {
    m_eventLoop.reset(new xop::EventLoop());
    std::string ip = xop::NetInterface::getLocalIPAddress(); //获取网卡ip地址
    std::string rtspUrl;
    m_rtspserver.reset(new xop::RtspServer(m_eventLoop.get(),ip,554));

    for(auto &session:m_ChannalSessionMap)
    {
        session.second.psession = xop::MediaSession::createNew(session.second.rtspUrlSuffxx);
        session.second.rtspUrl = "rtsp://" + ip + "/" + session.second.psession->getRtspUrlSuffix();
        session.second.psession->addMediaSource(xop::channel_0, xop::H264Source::createNew());
        session.second.psession->addMediaSource(xop::channel_1, xop::AACSource::createNew(44100,2));
        int channal = session.first;
        session.second.psession->setNotifyCallback([channal,this](xop::MediaSessionId sessionId, uint32_t numClients) {
            m_ChannalSessionMap[channal].clients = numClients; //获取当前媒体会话客户端数量
            std::cout << "[" << m_ChannalSessionMap[channal].rtspUrl << "]" << " Online: " << m_ChannalSessionMap[channal].clients << std::endl;
        });
        std::cout <<session.second.rtspUrl << std::endl;
        session.second.sessionId = m_rtspserver.get()->addMeidaSession(session.second.psession);
    }
    m_bstart = true;

#if 0
    m_psession = xop::MediaSession::createNew("live"); //创建一个媒体会话, url: rtsp://ip/live
    rtspUrl = "rtsp://" + ip + "/" + m_psession->getRtspUrlSuffix();
    // 添加音视频流到媒体会话, track0:h264, track1:aac
    m_psession->addMediaSource(xop::channel_0, xop::H264Source::createNew());
    m_psession->addMediaSource(xop::channel_1, xop::AACSource::createNew(44100,2));
    //session->startMulticast(); // 开启组播(ip,端口随机生成), 默认使用 RTP_OVER_UDP, RTP_OVER_RTSP
    // 设置通知回调函数。 在当前会话中, 客户端连接或断开会通过回调函数发起通知
    m_psession->setNotifyCallback([this, &rtspUrl](xop::MediaSessionId sessionId, uint32_t numClients) {
        m_clients = numClients; //获取当前媒体会话客户端数量
        std::cout << "[" << rtspUrl << "]" << " Online: " << m_clients << std::endl;
    });
    std::cout << rtspUrl << std::endl;
    xop::MediaSessionId sessionId = m_rtspserver.get()->addMeidaSession(m_psession); //添加session到RtspServer后, session会失效
    //server.removeMeidaSession(sessionId); //取消会话, 接口线程安全
#endif
    m_eventLoop.get()->loop(); //主线程运行 RtspServer
}

RtspLoop *RtspLoop::get_instance()
{
    if(_instance == nullptr)
    {
        _instance = new RtspLoop();
        pthread_t id;
        pthread_create(&id, nullptr,RtspLoop_fuction,(void*)(_instance));
    }
    return _instance;
}


RtspLoop::RtspLoop():m_bstart(false)
{
    ChannalSession session;
    memset(&session,sizeof(session),0);
    char buf[10];
    for(int i = 0; i < 4; i++)
    {
        session.clients = 0;
        session.psession = nullptr;
        session.rtspUrl = "";
        sprintf(buf,"channal_%d",i+1);
        session.rtspUrlSuffxx = buf;
        session.sessionId = 0;
        m_ChannalSessionMap.emplace(i,session);
    }
}

bool RtspLoop::putframe(int channel,xop::AVFrame frame) {
    /*
    if(m_bstart == true) {
        auto session = m_ChannalSessionMap.find(channal);
        if (session != m_ChannalSessionMap.end()) {
            if ((*session).second.clients > 0) {
                //std::cout<<"put frame"<<std::endl;
                frame.timestamp = frame.timestamp / 1000 * 90;
                m_rtspserver.get()->pushFrame((*session).second.sessionId, xop::channel_0, frame);
            }
        }
    }
     */
    if(channel < 0 || channel >4) {
        return false;
        std::cout<<"input channel error"<<std::endl;
    }
    if(m_bstart == true) {
        if(m_ChannalSessionMap[channel].clients > 0){
            frame.timestamp = frame.timestamp / 1000 * 90;
            m_rtspserver.get()->pushFrame(m_ChannalSessionMap[channel].sessionId, xop::channel_0, frame);
            return true;
        }
    }
    return false;
}
