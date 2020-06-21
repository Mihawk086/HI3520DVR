#ifndef HISI3520_HISI_INIT_H
#define HISI3520_HISI_INIT_H
#include "EventLoop.h"
extern "C"{
#include "sample_comm.h"
};
typedef struct VIO_PARAM
{
    VIDEO_NORM_E enNorm;
    PIC_SIZE_E enSize;
    SAMPLE_VI_MODE_E enViMode;
}VIO_PARAM;
typedef enum VO_MODE{
    VO_MODE_1,
    VO_MODE_2,
    VO_MODE_3,
    VO_MODE_4,
    VO_MODE_5,
}VO_MODE;
class Hisi_Init {
public:
    Hisi_Init() = delete;
    Hisi_Init(xop::EventLoop* loop);
    bool change_vo_mode();
    bool VIO_Init();
    bool Hisi_Exit();
    bool vb_conf();
private:
    bool Vo_Stop_Chn();
    bool Vo_Start_Chn();
    xop::EventLoop* _loop;
    VO_MODE vo_mode;
};
#endif //HISI3520_HISI_INIT_H
