#ifndef HISI3520_VENC_H
#define HISI3520_VENC_H
#include <map>

#include "EventLoop.h"
#include "Channel.h"
extern "C"{
#include "sample_comm.h"
};
typedef struct VENC_PARAM{
    VIDEO_NORM_E enNorm;
    PIC_SIZE_E enSize;
    HI_U32 u32Profile;
    SAMPLE_RC_E enRcMode;
}VENC_PARAM;
class Venc {
public:
    Venc() = delete;
    Venc(xop::EventLoop* loop);
    bool start_venc();
    bool stop_venc();
private:
    void read_stream_cb(VENC_CHN VencChn);
    bool init_user_vb();
    xop::EventLoop*_loop;
    VENC_PARAM _main_stream_param;
    VENC_PARAM _sub_stream_param;
    std::map<int,xop::ChannelPtr> _Channels;
};
#endif //HISI3520_VENC_H
