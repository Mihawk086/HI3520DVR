#include "Hisi_Init.h"

#include <cstring>
extern "C" {
#include "sample_comm.h"
}
bool Hisi_Init::vb_conf() {
    VB_CONF_S stVbConf;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_U32 u32ViChnCnt = 4;
    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;
    PIC_SIZE_E enSize = PIC_HD720;
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 10;
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret){
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return false;
    }
    return true;
}
bool Hisi_Init::Hisi_Exit() {
    VO_DEV VoDev = SAMPLE_VO_DEV_DHD0;
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P;
    HI_S32 s32VpssGrpCnt = 4;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
    VO_CHN VoChn;
    SAMPLE_VO_MODE_E enVoMode;
    HI_U32 u32WndNum;
    u32WndNum = 4;
    enVoMode = VO_MODE_4MUX;
    SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
    for(int i=0;i<u32WndNum;i++){
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
    return true;
}
bool Hisi_Init::VIO_Init() {
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_720P;
    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;
    HI_S32 s32VpssGrpCnt = 4;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SAMPLE_VO_MODE_E enVoMode;
    PIC_SIZE_E enSize = PIC_HD720;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    SIZE_S stSize;
    HI_U32 u32WndNum;
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("start vi failed!\n");
        Hisi_Exit();
        return false;
    }
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        Hisi_Exit();
        return false;
    }
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, NULL);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Start Vpss failed!\n");
        Hisi_Exit();
        return false;
    }
    VPSS_CHN_MODE_S stVpssChnMode;
    for (i=0; i<s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VPSS_CHN0, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("get Vpss chn mode failed!\n");
            Hisi_Exit();
            return false;
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
        if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("Set Vpss chn mode failed!\n");
            Hisi_Exit();
            return false;
        }
    }
    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        Hisi_Exit();
        return false;
    }
    printf("start vo HD0.\n");
    VoDev = SAMPLE_VO_DEV_DHD0;
    VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 4;
    enVoMode = VO_MODE_4MUX;
    stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    //stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
        Hisi_Exit();
        return false;
    }
    memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
        Hisi_Exit();
        return false;
    }
    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
        Hisi_Exit();
        return false;
    }
    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
        Hisi_Exit();
        return false;
    }
    for(i=0;i<u32WndNum;i++) {
        VoChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
        if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("Start VO failed!\n");
            Hisi_Exit();
            return false;
        }
    }
    return true;
}
Hisi_Init::Hisi_Init(xop::EventLoop *loop): _loop(loop){
}
bool Hisi_Init::change_vo_mode() {
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
    if (HI_SUCCESS != s32Ret){
        SAMPLE_PRT("Start VO failed!\n");
    }
    s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
    if (HI_SUCCESS != s32Ret){
        SAMPLE_PRT("Start VO failed!\n");
    }
    return false;
}
bool Hisi_Init::Vo_Stop_Chn() {
    HI_S32 s32Ret = HI_SUCCESS;
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
    for (int i = 0; i < 4; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
        if (s32Ret != HI_SUCCESS){
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return false;
        }
    }
    return true;
}
bool Hisi_Init::Vo_Start_Chn() {
    return true;
}
