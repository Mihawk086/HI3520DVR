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
#include "vivo_init.h"
HI_S32 SAMPLE_VIO_8_1080P_DUAL(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P;
    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;
    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN2;
    VO_DEV VoDev;
	VO_LAYER VoLayer;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SAMPLE_VO_MODE_E enVoMode;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;
    HI_U32 u32WndNum;
	VO_WBC VoWbc;
    VO_WBC_ATTR_S stWbcAttr;    
    VO_WBC_SOURCE_S stWbcSource;
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    stVbConf.u32MaxPoolCnt = 128;
    //todo: vb=15
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_8X1080P_0;
    }
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_8X1080P_0;
    }
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_8X1080P_0;
    }
	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_8X1080P_1;
    }
    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_8X1080P_2;
    }
    printf("start vo HD0.\n");
    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;
    stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8X1080P_3;
	}
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8X1080P_3;
	}
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8X1080P_3;
	}
    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_8X1080P_4;
    }
#ifdef HDMI_SUPPORT
    if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
        {
            SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
            goto END_8X1080P_4;
        }
    }
#endif
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_8X1080P_4;
        }
    }
	printf("start vo SD0: wbc from hd0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;
	stVoPubAttr.enIntfSync = VO_OUTPUT_NTSC;
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8X1080P_4;
	}
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8X1080P_4;
	}
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8X1080P_4;
	}
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8X1080P_5;
	}
	VoWbc = SAMPLE_VO_WBC_BASE;
    stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
    stWbcSource.u32SourceId = SAMPLE_VO_DEV_DHD0;
    s32Ret = SAMPLE_COMM_WBC_BindVo(VoWbc,&stWbcSource);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_5;
    }
    s32Ret = SAMPLE_COMM_VO_GetWH(VO_OUTPUT_PAL, \
		&stWbcAttr.stTargetSize.u32Width, \
		&stWbcAttr.stTargetSize.u32Height, \
		&stWbcAttr.u32FrameRate);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_5;
    }
	stWbcAttr.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VO_StartWbc(VoWbc,&stWbcAttr);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_6;
    }
    s32Ret = SAMPLE_COMM_VO_BindVoWbc(SAMPLE_VO_DEV_DHD0,SAMPLE_VO_LAYER_VSD0,0);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_6;
    }
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;
    while(1)
    {    
        printf("press 'q' to exit this sample.\n");
        ch = getchar();
        if(10 == ch)
        {
            continue;
        }
        getchar();
        if ('q' == ch)
        {
            break;
        }
        else
        {
            SAMPLE_PRT("input invaild! please try again.\n");
            continue;
        }	
    }
END_8X1080P_6:
	SAMPLE_COMM_VO_UnBindVoWbc(SAMPLE_VO_DEV_DSD0, 0);
	HI_MPI_VO_DisableWbc(SAMPLE_VO_DEV_DHD0);
END_8X1080P_5:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	VoChn = 0;
	enVoMode = VO_MODE_9MUX;
    SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);
END_8X1080P_4:
#ifdef HDMI_SUPPORT
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
#endif
    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;   
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
	SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);
END_8X1080P_3:
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8X1080P_2:
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8X1080P_1:
    SAMPLE_COMM_VI_Stop(enViMode);
END_8X1080P_0:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

HI_S32 SAMPLE_VIO_8_720P(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_720P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;
	HI_U32 u32ViChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;
	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;
	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	VO_PUB_ATTR_S stVoPubAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				PIC_HD720, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 128;
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 10;
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_8_720P_0;
	}
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_720P_0;
	}
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD720, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_720P_1;
	}
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_8_720P_1;
	}
    VPSS_CHN_MODE_S stVpssChnMode;
    for (i=0; i<s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VPSS_CHN0, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("get Vpss chn mode failed!\n");
            goto END_8_720P_2;
        }
        memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
        stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
        stVpssChnMode.u32Width  = stSize.u32Width;
        stVpssChnMode.u32Height = stSize.u32Height;
        stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
        stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
        s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VPSS_CHN0, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Set Vpss chn mode failed!\n");
            goto END_8_720P_2;
        }
    }
	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_720P_2;
	}
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_4MUX;
	stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_720P_3;
	}
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_720P_3;
	}
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_720P_3;
	}
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_720P_4;
	}
#ifdef HDMI_SUPPORT
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_720P_4;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_4;
		}
	}
		return 0;
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_4MUX;
	stVoPubAttr.enIntfSync = VO_OUTPUT_NTSC;
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_720P_4;
	}
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_720P_4;
	}
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_720P_4;
	}
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_720P_5;
	}
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
	}
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;
	while(1)
	{
		enPreVoMode = enVoMode;
		printf("please choose preview mode, press 'q' to exit this sample.\n"); 
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
		printf("\tq) quit\n");
		ch = getchar();
        if(10 == ch)
        {
            continue;
        }
		getchar();
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);
		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
	}
END_8_720P_5:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_720P_4:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;	
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_720P_3:	//vi unbind vpss
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_720P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_720P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_720P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


