#ifndef HISI3520_OSD_H
#define HISI3520_OSD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <EventLoop.h>
extern "C"{
#include "sample_comm.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "loadbmp.h"
}
class Osd {
public:
    Osd() = delete;
    Osd(xop::EventLoop * loop);
    bool time_osd_init();
    bool time_osd_destroy();
    bool start_time_osd();
    bool stop_time_osd();
private:
    RGN_HANDLE _time_handle;
    xop::EventLoop* _loop;
    xop::TimerId _time_osd_id;
};
#endif
