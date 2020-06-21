//
// Created by fuck on 18-11-26.
//

#include "RtspPusherLoop.h"

#define PUSH_TEST "rtsp://192.168.1.101:554/test"

static void* RtspPusherLoop_fuction(void* p)
{
    RtspPusherLoop* ptr = static_cast<RtspPusherLoop*>(p);
    ptr->loop();
}

RtspPusherLoop* RtspPusherLoop::_instance = NULL;

bool RtspPusherLoop::Putframe(xop::AVFrame frame) {
    _pusher_ptr.get()->pushFrame(_sessionID,xop::channel_0,frame);
    return false;
}

RtspPusherLoop::RtspPusherLoop() {
    _loop_ptr.reset(new xop::EventLoop());
    _pusher_ptr.reset(new xop::RtspPusher(_loop_ptr.get()));
    _mediasession_ptr.reset(xop::MediaSession::createNew());
    _mediasession_ptr.get()->addMediaSource(xop::channel_0, xop::H264Source::createNew());
    _mediasession_ptr.get()->addMediaSource(xop::channel_1, xop::AACSource::createNew(44100,2));
    _sessionID = _pusher_ptr.get()->addMeidaSession(_mediasession_ptr.get());
}

RtspPusherLoop *RtspPusherLoop::Instance() {
    if(_instance == NULL){
        _instance = new RtspPusherLoop();
        pthread_t id;
        pthread_create(&id,NULL,RtspPusherLoop_fuction,(void*)(_instance));
    }
}

void RtspPusherLoop::loop() {
    if(!_pusher_ptr.get()->openUrl(PUSH_TEST)){
        std::cout << "Open " << PUSH_TEST << " failed." << std::endl; // 连接服务器超时
        return ;
    }
    std::cout << "Push stream to " << PUSH_TEST << " ..." << std::endl;
    _loop_ptr.get()->loop();
}

