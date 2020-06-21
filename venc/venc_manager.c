#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include "sample_comm.h"
#include "venc_manager.h"
#include "ringfifo.h"
    #define START 1
#define STOP 0
typedef struct venc_getstream_s
{
     int bThreadStart;
     int  s32Cnt;
}venc_getstream_s;
static venc_getstream_s gs_stPara;
static void* VencGetStreamThread(void *p)
{
    int maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    int VencFd[VENC_MAX_CHN_NUM];
    VENC_STREAM_S stStream;
    int ret;
    venc_getstream_s *pstPara;
    int s32ChnTotal;
    int i;
    VENC_CHN_STAT_S stStat;
    pstPara = (venc_getstream_s*)p;
    s32ChnTotal = pstPara->s32Cnt;
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
    while (pstPara->bThreadStart == HI_TRUE )
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
                    if(i == 2){
                    }
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
static int VencSet(void)
{
    VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_AUTO;
    HI_U32 u32ViChnCnt = 4;
    PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_JPEG,PT_H264};
    PIC_SIZE_E enSize[3] = {PIC_HD720, PIC_HD720,PIC_CIF};
    HI_U32 u32Profile = 2; /*0: baseline; 1:MP; 2:HP 3:svc-t */
    VPSS_CHN VpssChn;
    VPSS_GRP VpssGrp;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32  u32BlkSize;
    HI_U32 u32BlkCnt = 4*2;
    u32BlkSize = 1024*1024;
    enRcMode = SAMPLE_RC_VBR;
    for (i=0 ; i<u32ViChnCnt; i++)
    {
        VencChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad[0],\
                                       gs_enNorm, PIC_HD720, enRcMode,u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_3;
        }
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_CHN0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_3;
        }
    }
    VB_POOL VbPool;
    VbPool = HI_MPI_VB_CreatePool(u32BlkSize,u32BlkCnt,"anonymous");
    if ( VB_INVALID_POOLID == VbPool )
    {
        printf("create vb err\n");
        return HI_FAILURE;
    }
    for (i=0 ; i<u32ViChnCnt; i++)
    {
        VencChn = i;
        HI_MPI_VENC_AttachVbPool(VencChn, VbPool);
    }
    return 0;
END_VENC_8D1_3:
    for (i=0; i<u32ViChnCnt; i++)
    {
        VencChn = i;
        VpssGrp = i;
        VpssChn = VPSS_CHN0;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
    printf("exit enc pthread\n");
    pthread_exit(0);
    return NULL;
}
void* SubStream(void* arg)
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
    while (1)
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
                    if(i == 2)
                    {
                        HisiPutH264DataToBuffer(&stStream);
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
int VencInit()
{
    pthread_t t1,t2,t3;
    HI_S32 s32Ret = HI_SUCCESS;
    gs_stPara.bThreadStart = HI_TRUE;
    gs_stPara.s32Cnt = 4;
    VencSet();
    s32Ret = pthread_create(&t1, 0, VencGetStreamThread, (HI_VOID*)&gs_stPara);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
    }
    return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


