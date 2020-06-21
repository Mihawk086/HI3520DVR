#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
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
#include "sample_comm.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h" 
#include "loadbmp.h"
static char tmp[25];
static TTF_Font *font;  
static SDL_Surface *text, *temp;  
static SDL_Rect bounds;
static BITMAP_S stBitmap;
static HI_S32 SAMPLE_RGN_DestroyRegion(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        s32Ret = HI_MPI_RGN_Destroy(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_Destroy failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }
    return HI_SUCCESS;
}
static BITMAP_S SDL_OSDtoBMP(char *tmp_sys_time)
{
    int s32Ret;
    static unsigned char Count=0;
    if (TTF_Init() < 0 )
    {  
            fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());  
            SDL_Quit();
    }
    font = TTF_OpenFont("./Vera.ttf", 48);
    if ( font == NULL )
    {  
            fprintf(stderr, "Couldn't load %d pt font from %s: %s\n",  
            "ptsize", 18, SDL_GetError());  
    }  
    SDL_Color forecol=   { 0xff, 0xff, 0xff, 0xff };  
    text = TTF_RenderUTF8_Solid(font, tmp_sys_time, forecol);
    SDL_PixelFormat *fmt;
    fmt = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));
    memset(fmt,0,sizeof(SDL_PixelFormat));
    fmt->BitsPerPixel = 16;
    fmt->BytesPerPixel = 2;
    fmt->colorkey = 0xffffffff;
    fmt->alpha = 0xff;
    SDL_Surface *temp = SDL_ConvertSurface(text,fmt,0);
    SDL_SaveBMP(temp, "sys_time.bmp");
    stBitmap.u32Width = temp->pitch/2;
    stBitmap.u32Height = temp->h;
    stBitmap.pData= temp->pixels;
    stBitmap.enPixelFormat= PIXEL_FORMAT_RGB_1555;
    s32Ret = HI_MPI_RGN_SetBitMap(0,&stBitmap);
    if(s32Ret != HI_SUCCESS) {
        SAMPLE_PRT("HI_MPI_RGN_SetBitMap failed with %#x!\n", s32Ret);
    }
    SDL_FreeSurface(text);  
    SDL_FreeSurface(temp);
    TTF_CloseFont(font);  
    TTF_Quit();  
    return stBitmap;
}
static HI_S32 SAMPLE_RGN_CreateOverlayEx()
{
    HI_S32 s32Ret = HI_FAILURE;
    RGN_HANDLE OverlayExHandle = 0;
    RGN_ATTR_S stOverlayExAttr;
    MPP_CHN_S stOverlayExChn;
    RGN_CHN_ATTR_S stOverlayExChnAttr;
	stOverlayExAttr.enType = OVERLAYEX_RGN;
	stOverlayExAttr.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stOverlayExAttr.unAttr.stOverlayEx.u32BgColor =  0x00;
	stOverlayExAttr.unAttr.stOverlayEx.stSize.u32Height = 50;
	stOverlayExAttr.unAttr.stOverlayEx.stSize.u32Width= 750;
    s32Ret = HI_MPI_RGN_Create(OverlayExHandle, &stOverlayExAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_Create failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    stOverlayExChnAttr.enType = OVERLAYEX_RGN;
	stOverlayExChnAttr.bShow = HI_TRUE;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 0;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 0;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = 0;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = 255;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.u32Layer = 1;
    stOverlayExChn.enModId = HI_ID_VPSS;
    stOverlayExChn.s32DevId = 0;
    stOverlayExChn.s32ChnId = 1;
	int i ;
	for(i = 0 ; i < 4 ;i++)
	{
		stOverlayExChn.s32DevId = i;
		s32Ret = HI_MPI_RGN_AttachToChn(OverlayExHandle,&stOverlayExChn,&stOverlayExChnAttr);
	    if(HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("HI_MPI_RGN_AttachToChn failed with %#x!\n", s32Ret);
	        return HI_FAILURE;
	    }
	}
	stOverlayExChn.s32ChnId = 0;
	for(i = 0 ; i < 4 ;i++)
	{
		stOverlayExChn.s32DevId = i;
		s32Ret = HI_MPI_RGN_AttachToChn(OverlayExHandle,&stOverlayExChn,&stOverlayExChnAttr);
	    if(HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("HI_MPI_RGN_AttachToChn failed with %#x!\n", s32Ret);
	        return HI_FAILURE;
	    }
	}
	return HI_SUCCESS;
}
static void* SAMPLE_RGN_AddOsdToVpss( void* arg)
{
    RGN_HANDLE Handle;
    HI_S32 s32Ret = HI_SUCCESS;
    BITMAP_S stBitmap;
    Handle    = 0;
    char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    time_t timep;
    struct tm *p;
    char time_buf[18]="\0";
	while(1)
    {
	    time(&timep);
	    p=localtime(&timep);
	    sprintf(time_buf, "%04d/%02d/%02d-%02d:%02d:%02d-%s", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,\
	                    p->tm_hour, p->tm_min, p->tm_sec, wday[p->tm_wday]);
	    stBitmap = SDL_OSDtoBMP(time_buf);
	    usleep(333333);
    }
    if(NULL != stBitmap.pData)
    {
            free(stBitmap.pData);
            stBitmap.pData = NULL;
    }
}

static HI_S32 OsdInitial(HI_VOID)
{
    RGN_HANDLE Handle;  
    HI_S32 u32RgnNum;
    HI_S32 s32Ret = HI_SUCCESS;
    Handle    = 0;
    u32RgnNum = 1;
    s32Ret = SAMPLE_RGN_DestroyRegion(Handle, u32RgnNum);
	s32Ret = SAMPLE_RGN_CreateOverlayEx();
    //s32Ret = SAMPLE_RGN_CreateOverlayExForVpss(Handle, u32RgnNum);  
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayExForVpss failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}
int region_init()
{
	OsdInitial();
	pthread_t id;
	pthread_create(&id, NULL, SAMPLE_RGN_AddOsdToVpss, NULL);
	return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

