#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "Gui_Manager.h"
extern "C" {
#include "sample_comm.h"
#include "hifb.h"
}
#define HI_WIDTH_SYNC 1280
#define HI_HEIGHT_SYNC 720
static HIFB_POINT_S g_stPoint = {448,180};
static struct fb_bitfield g_r32 = {16,8, 0};
static struct fb_bitfield g_g32 = {8, 8, 0};
static struct fb_bitfield g_b32 = {0, 8, 0};
static struct fb_bitfield g_a32 = {24, 8, 0};
Gui_Manager::Gui_Manager(xop::EventLoop *loop) :_loop(loop){
}
bool Gui_Manager::Init() {
    int fdGUI = false;
    HI_MPI_VO_UnBindGraphicLayer(GRAPHICS_LAYER_HC0, 0);
    if (HI_SUCCESS != HI_MPI_VO_BindGraphicLayer(GRAPHICS_LAYER_HC0, 0)) {
        printf("%s: Graphic Bind to VODev failed!,line:%d\n", __FUNCTION__,  __LINE__);
        return false;
    }
    fdGUI = open("/dev/fb0", O_RDWR, 0);
    if(fdGUI <0) {
        perror("open");
        return false;
    }
    HI_BOOL bShow = HI_FALSE;
    if (ioctl(fdGUI, FBIOPUT_SHOW_HIFB, &bShow) < 0)
    {
        printf("FBIOPUT_SHOW_HIFB failed!\n");
        return false;
    }
    HIFB_ALPHA_S stAlpha;
    memset(&stAlpha, 9, sizeof(stAlpha));
    stAlpha.bAlphaEnable = HI_TRUE;
    stAlpha.bAlphaChannel = HI_TRUE;
    stAlpha.u8Alpha0 = 0xff;
    stAlpha.u8Alpha1 = 0xff;
    stAlpha.u8GlobalAlpha = 0xff;
    HIFB_POINT_S stPoint = {320,180};
    if (ioctl(fdGUI, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0) {
        printf("Set alpha failed!\n");
        return false;
    }
    if (ioctl(fdGUI, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
    {
        printf("set screen original show position failed!\n");
        return false;
    }
    struct fb_var_screeninfo var;
    if (ioctl(fdGUI, FBIOGET_VSCREENINFO, &var) < 0){
        printf("Get variable screen info failed!\n");
        return false;
    }
    var.xres_virtual = HI_WIDTH_SYNC;
    var.yres_virtual = HI_HEIGHT_SYNC;
    var.xres = HI_WIDTH_SYNC;
    var.yres = HI_HEIGHT_SYNC;
    var.transp= g_a32;
    var.red = g_r32;
    var.green = g_g32;
    var.blue = g_b32;
    var.bits_per_pixel = 32;
    var.activate = FB_ACTIVATE_NOW;
    if (ioctl(fdGUI, FBIOPUT_VSCREENINFO, &var) < 0){
        printf("Put variable screen info failed!\n");
        return false;
    }
    bShow = HI_TRUE;
    if(ioctl(fdGUI, FBIOPUT_SHOW_HIFB, &bShow) !=0) {
        printf("FBIOPUT_SHOW_HIFB failed\n");
        return false;
    }
    _fd = fdGUI;
    return true;
}
bool Gui_Manager::Exit() {
    if(_fd > 0) {
        close(_fd);
    }
    HI_MPI_VO_UnBindGraphicLayer(GRAPHICS_LAYER_HC0, 0);
    return true;
}
