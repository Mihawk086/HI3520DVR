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
#include <hifb.h>
#include <sys/mman.h>

#include "sample_comm.h"
#include "gui_manager.h"

#define HI_WIDTH_SYNC 1000
#define HI_HEIGHT_SYNC 1000

static struct fb_bitfield g_r32 = {16,8, 0};
static struct fb_bitfield g_g32 = {8, 8, 0};
static struct fb_bitfield g_b32 = {0, 8, 0};
static struct fb_bitfield g_a32 = {24, 8, 0};
static int fdGUI = -1;

int GuiInit()
{
	HI_MPI_VO_UnBindGraphicLayer(GRAPHICS_LAYER_HC0, 0);
	if (HI_SUCCESS != HI_MPI_VO_BindGraphicLayer(GRAPHICS_LAYER_HC0, 0)) {
	     printf("%s: Graphic Bind to VODev failed!,line:%d\n", __FUNCTION__,  __LINE__);
	    return -1;
	}
	fdGUI = open("/dev/fb0", O_RDWR, 0);
	if(fdGUI <0) {
	    perror("open");
	   return -1;
	}
	HI_BOOL bShow = HI_FALSE;
	if (ioctl(fdGUI, FBIOPUT_SHOW_HIFB, &bShow) < 0) 
	{
	    printf("FBIOPUT_SHOW_HIFB failed!\n");
	    return -1;
	}
	HIFB_ALPHA_S stAlpha;
	memset(&stAlpha, 9, sizeof(stAlpha));
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_TRUE;
	stAlpha.u8Alpha0 = 0xff;
	stAlpha.u8Alpha1 = 0xff;
	stAlpha.u8GlobalAlpha = 0xff;
	HIFB_POINT_S stPoint = {200,100};
	
	if (ioctl(fdGUI, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0) {
	    printf("Set alpha failed!\n");
	    return -1;
	}

	/*2. set the screen original position*/
	if (ioctl(fdGUI, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
	{
		printf("set screen original show position failed!\n");
		return -1;
	}
	
	struct fb_var_screeninfo var;
	if (ioctl(fdGUI, FBIOGET_VSCREENINFO, &var) < 0){
	    printf("Get variable screen info failed!\n");
	    return -1;
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
	    return -1;
	}
	bShow = HI_TRUE;
	if(ioctl(fdGUI, FBIOPUT_SHOW_HIFB, &bShow) !=0) {
		printf("FBIOPUT_SHOW_HIFB failed\n");
		return -1;
	}
/*
	int screen_size;
	unsigned char *fbmem;
	screen_size = HI_WIDTH_SYNC * HI_HEIGHT_SYNC;
	fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdGUI, 0);
	if (fbmem == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}
	memset(fbmem, 0xfffffff, screen_size);
*/
	//close(fdGUI);	
}

int GuiExit()
{
	if(fdGUI > 0)
	{
		close(fdGUI);
	}
	HI_MPI_VO_UnBindGraphicLayer(GRAPHICS_LAYER_HC0, 0);
}

