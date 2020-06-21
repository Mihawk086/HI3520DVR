//
// Created by fuck on 18-11-26.
//

#ifndef HISI3520_RTSPPUSHERLOOP_H
#define HISI3520_RTSPPUSHERLOOP_H


#include <memory>


#include "EventLoop.h"
#include "media.h"
#include "RtspPusher.h"
#include "MediaSession.h"

class RtspPusherLoop {
public:
    static RtspPusherLoop* Instance();
    static RtspPusherLoop* _instance;
    void loop();
    bool Putframe(xop::AVFrame);
private:
    RtspPusherLoop();
    std::shared_ptr<xop::EventLoop> _loop_ptr;
    std::shared_ptr<xop::RtspPusher> _pusher_ptr;
    std::shared_ptr<xop::MediaSession> _mediasession_ptr;
    xop::MediaSessionId _sessionID;
};


#endif //HISI3520_RTSPPUSHERLOOP_H
