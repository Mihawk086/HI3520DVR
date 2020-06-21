#include "cvencmanager.h"
#include "csaveinterface.h"
#include "CRingFifoInstance.h"
#include "RecordLoop.h"
#include "media.h"
#include "RtspLoop.h"
extern "C"{
    #include "ringfifo.h"
}
CVencManager::CVencManager(){
}
CVencManager::~CVencManager(){
}
CVencManager* CVencManager::get_instance(){
    static CVencManager instance;
    return &instance;
}
bool CVencManager::ReadConfig()
{
    m_pConfig = CConfig::get_instance();
    m_pConfig->ReadJSON();
    T_RecordMenu recordmenu = m_pConfig->GetRecordMenu();
    if(recordmenu.standard == Standard_PAL) {
        m_enNorm = VIDEO_ENCODING_MODE_PAL;
    }
    else if(recordmenu.standard == Standard_NTSC) {
        m_enNorm = VIDEO_ENCODING_MODE_NTSC;
    }
    else {
        m_enNorm = VIDEO_ENCODING_MODE_PAL;
    }
    switch (recordmenu.standard) {
    case Standard_PAL:
        m_enMainSize = PIC_D1;
        m_enSubSize  = PIC_CIF;
        break;
    case Standard_NTSC:
        m_enMainSize = PIC_D1;
        m_enSubSize  = PIC_CIF;
        break;
    case Standard_720P:
        m_enMainSize = PIC_HD720;
        m_enSubSize  = PIC_D1;
    case Standard_1080P:
        m_enMainSize = PIC_HD1080;
        m_enSubSize  = PIC_D1;
    default:
        break;
    }
}
bool CVencManager::Setting()
{
    VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
    PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD720,PIC_D1};
    HI_U32 u32Profile = 2; /*0: baseline; 1:MP; 2:HP 3:svc-t */
    SAMPLE_RC_E enRcMode;
    HI_U32 u32ViChnCnt = 4;
    VPSS_GRP VpssGrp;
    VENC_CHN VencChn;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    VB_POOL VbPool;
    HI_U32 u32BlkSize = 1024*1024 ;
    HI_U32 u32BlkCnt = 16;
    enRcMode = SAMPLE_RC_VBR;
    for (i=0 ; i<u32ViChnCnt; i++)
    {
        VencChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, PT_H264,\
                                       gs_enNorm, PIC_HD720, enRcMode,u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
        }
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_CHN0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
        }
    }
    for (i=0 ; i<u32ViChnCnt; i++)
    {
        VencChn = i+4;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, PT_H264,\
                                       gs_enNorm, PIC_D1, enRcMode,u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
        }
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_CHN0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
        }
    }
    VbPool = HI_MPI_VB_CreatePool(u32BlkSize,u32BlkCnt,"anonymous");
    if ( VB_INVALID_POOLID == VbPool )
    {
        printf("create vb err\n");
        return HI_FAILURE;
    }
    for (i=0 ; i< (u32ViChnCnt*2); i++)
    {
        VencChn = i;
        HI_MPI_VENC_AttachVbPool(VencChn, VbPool);
    }
    return true;
}
int CVencManager::VencStart()
{
    pthread_t t1,t2;
    HI_S32 s32Ret = HI_SUCCESS;
    Setting();
    s32Ret = pthread_create(&t1, 0, CVencManager::StartMainStream, (void *)this);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
    }
    s32Ret = pthread_create(&t2, 0, CVencManager::StartSubStream, (void*)this);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
    }
    m_idMainThread = t1;
    m_idSubThread = t2;
    m_bMainThreadStart = true;
    m_bSubThreadStart = true;
}
void* CVencManager::onSubStream(void)
{
    int maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    int VencFd[4];
    VENC_STREAM_S stStream;
    VENC_CHN_STAT_S stStat;
    int ret;
    int s32ChnTotal;
    int i;
    xop::AVFrame frame;
    m_bSubThreadStart = true;
    s32ChnTotal = 4;
    for (i = 0; i < s32ChnTotal; i++)
    {
        VencFd[i] = HI_MPI_VENC_GetFd(i+4);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                   VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }
    while (m_bSubThreadStart == true)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
        for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    memset(&stStream, 0, sizeof(stStream));
                    ret = HI_MPI_VENC_Query((i+4), &stStat);
                    if (HI_SUCCESS != ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, ret);
                        break;
                    }
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }
                    stStream.u32PackCount = stStat.u32CurPacks;
                    ret = HI_MPI_VENC_GetStream((i+4), &stStream, HI_TRUE);
                    if (HI_SUCCESS != ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                               ret);
                        break;
                    }
                    ret = HI_MPI_VENC_ReleaseStream((i+4), &stStream);
                    if (HI_SUCCESS != ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    }
}
int CVencManager::VencExit()
{
    m_bMainThreadStart = false;
    m_bSubThreadStart = false;
    pthread_join(m_idMainThread,0);
    pthread_join(m_idSubThread,0);
    int i = 0;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VENC_CHN VencChn;
    for (i = 0; i< 4 ; i++)
    {
        VencChn = i;
        VpssGrp = i;
        VpssChn = VPSS_CHN0;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
    for (i = 0; i< 4 ; i++)
    {
        VencChn = i+4;
        VpssGrp = i;
        VpssChn = VPSS_CHN0;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
    return 0;
}
static int HisiPutH264DataToAVFrame(VENC_STREAM_S *pstStream,xop::AVFrame &frame)
{
    int i,j;
    int len = 0, off = 0, off2 = 0;
    unsigned char *pstr;
    int packlen = 0;
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        len += (pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
    }
    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        memcpy(frame.buffer.get()+off,pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
               pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
        pstr = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
        packlen = pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
        off2 = off;
        off +=  pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
        if(pstStream->pstPack[i].DataType.enH264EType == H264E_NALU_ISLICE)
        {
            frame.Iframe_len = packlen;
            frame.Iframe_offent = off2;
        }
        if(pstr[4] == 0x67)
        {
            if(packlen < 50) {
                memcpy(frame.sps,pstr,packlen);
                frame.sps_len = packlen;
            }
            else {
                printf("sps error\n");
            }
        }
        if(pstr[4] == 0x68)
        {
            if(packlen < 50) {
                memcpy(frame.pps,pstr,packlen);
                frame.pps_len = packlen;
            }
            else {
                printf("pps error\n");
            }
        }
    }
    if(pstStream->stH264Info.enRefType == BASE_IDRSLICE){
        frame.type = VIDEO_FRAME_I;
    }else{
        frame.type = VIDEO_FRAME_P;
    }
    frame.size = len;
    frame.timestamp = pstStream->pstPack->u64PTS;
    return 0;
}
void* CVencManager::onMainStream(void){
    int maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    int VencFd[4];
    VENC_STREAM_S stStream;
    VENC_CHN_STAT_S stStat;
    int ret;
    int s32ChnTotal;
    int i,j;
    int len = 0;
    xop::AVFrame frame;
    unsigned long long lastframe = 0;
    unsigned long long thisframe = 0;
    int interval = 0;
    int count = 0;

    m_bMainThreadStart = true;
    s32ChnTotal = 4;
    for (i = 0; i < s32ChnTotal; i++)
    {
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                   VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }
    while (m_bMainThreadStart == true)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }
        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
        for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    memset(&stStream, 0, sizeof(stStream));
                    ret = HI_MPI_VENC_Query(i, &stStat);
                    if (HI_SUCCESS != ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, ret);
                        break;
                    }
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }
                    stStream.u32PackCount = stStat.u32CurPacks;
                    ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                               ret);
                        break;
                    }
                    if(i ==2) {
                        thisframe = stStream.pstPack->u64PTS;
                        interval = thisframe - lastframe;
                        if (interval > 80000) {
                            printf("interval = %d count = %d\n", interval,++count);
                        }
                        lastframe = stStream.pstPack->u64PTS;
                    }
                    len = 0;
                    for (j = 0; j < stStream.u32PackCount; j++) {
                        len += (stStream.pstPack[j].u32Len - stStream.pstPack[j].u32Offset);
                    }
                    frame.size = len;
                    frame.buffer.reset(new char[len],[](char *p){
                        delete[] p;
                    });
                    HisiPutH264DataToAVFrame(&stStream, frame);
                    RecordLoop::Instance()->putframe(i+1,frame);
                    RtspLoop::get_instance()->putframe(i,frame);
                    frame.buffer.reset();
                    ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    }
}
void* CVencManager::StartMainStream(void* para)
{
    CVencManager *ptr = static_cast<CVencManager *>(para);
    ptr->onMainStream();
}
void* CVencManager::StartSubStream(void* para)
{
    CVencManager *ptr = static_cast<CVencManager *>(para);
    ptr->onSubStream();
}


