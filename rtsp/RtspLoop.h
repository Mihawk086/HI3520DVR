//
// Created by fuck on 18-8-24.
//

#ifndef HISI3520_RTSPLOOP_H
#define HISI3520_RTSPLOOP_H

#include "RtspPusher.h"
#include "RtspServer.h"
#include "xop.h"
#include "xop/NetInterface.h"
#include "media.h"

#include <thread>
#include <memory>
#include <iostream>
#include <string>

typedef struct ChannalSession
{
    int clients;
    xop::MediaSession* psession;
    xop::MediaSessionId sessionId;
    std::string rtspUrl;
    std::string rtspUrlSuffxx;
}ChannalSession;

class RtspLoop {
public:
    static RtspLoop* get_instance();

    static RtspLoop* _instance;

    bool putframe(int channal,xop::AVFrame frame);

    bool loop();

private:
    RtspLoop();

    bool m_bstart;

    std::shared_ptr<xop::EventLoop> m_eventLoop;

    std::shared_ptr<xop::RtspServer> m_rtspserver;

    std::unordered_map<int,ChannalSession> m_ChannalSessionMap;
};


#endif //HISI3520_RTSPLOOP_H
