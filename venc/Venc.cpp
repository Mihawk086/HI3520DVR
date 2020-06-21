#include "Venc.h"
#include "media.h"
#include "RtspLoop.h"
#include "RecordLoop.h"
#include "RtspPusherLoop.h"
#include <cstring>
static int HisiPutH264DataToAVFrame(VENC_STREAM_S *pstStream,xop::AVFrame &frame)
{
    int i,j;
    int len = 0, off = 0, off2 = 0;
    unsigned char *pstr;
    int packlen = 0;
    for (i = 0; i < pstStream->u32PackCount; i++){
        len += (pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
    }
    for (i = 0; i < pstStream->u32PackCount; i++){
        memcpy(frame.buffer.get()+off,pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
               pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
        pstr = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
        packlen = pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
        off2 = off;
        off +=  pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
        if(pstStream->pstPack[i].DataType.enH264EType == H264E_NALU_ISLICE){
            frame.Iframe_len = packlen;
            frame.Iframe_offent = off2;
        }
        if(pstr[4] == 0x67)
        {
            if(packlen < 50) {
                memcpy(frame.sps,pstr,packlen);
                frame.sps_len = packlen;
            }else {
                printf("sps error\n");
            }
        }
        if(pstr[4] == 0x68)
        {
            if(packlen < 50) {
                memcpy(frame.pps,pstr,packlen);
                frame.pps_len = packlen;
            }else {
                printf("pps error\n");
            }
        }
    }
    if(pstStream->stH264Info.enRefType == BASE_IDRSLICE) {
        frame.type = VIDEO_FRAME_I;
    }else {
        frame.type = VIDEO_FRAME_P;
    }
    frame.size = len;
    frame.timestamp = pstStream->pstPack->u64PTS;
    return 0;
}
Venc::Venc(xop::EventLoop *loop) :_loop(loop){
    _main_stream_param.u32Profile = 2;
    _main_stream_param.enNorm = VIDEO_ENCODING_MODE_NTSC;
    _main_stream_param.enSize = PIC_HD720;
    _main_stream_param.enRcMode = SAMPLE_RC_VBR;
    _sub_stream_param.u32Profile = 2;
    _sub_stream_param.enNorm = VIDEO_ENCODING_MODE_NTSC;
    _sub_stream_param.enSize = PIC_D1;
    _sub_stream_param.enRcMode = SAMPLE_RC_VBR;
}
bool Venc::init_user_vb() {
    VB_POOL VbPool;
    HI_U32 u32BlkSize = 1024*1024 ;
    HI_U32 u32BlkCnt = 16;
    VENC_CHN VencChn;
    VbPool = HI_MPI_VB_CreatePool(u32BlkSize,u32BlkCnt,"anonymous");
    if ( VB_INVALID_POOLID == VbPool ){
        printf("create vb err\n");
        return HI_FAILURE;
    }
    for (int i = 0 ; i < 8; i++){
        VencChn = i;
        HI_MPI_VENC_AttachVbPool(VencChn, VbPool);
    }
    return true;
}
bool Venc::start_venc() {
    HI_U32 u32ViChnCnt = 4;
    VPSS_GRP VpssGrp;
    VENC_CHN VencChn;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    for (i =0 ; i<u32ViChnCnt; i++){
        VencChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, PT_H264,\
                                       _main_stream_param.enNorm, _main_stream_param.enSize,
                                        _main_stream_param.enRcMode,_main_stream_param.u32Profile);
        if (HI_SUCCESS != s32Ret){
            SAMPLE_PRT("Start Venc failed!\n");
        }
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_CHN0);
        if (HI_SUCCESS != s32Ret){
            SAMPLE_PRT("Start Venc failed!\n");
        }
    }
    for (i=0 ; i<u32ViChnCnt; i++){
        VencChn = i+4;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, PT_H264,\
                                       _sub_stream_param.enNorm, _main_stream_param.enSize,
                                        _sub_stream_param.enRcMode,_sub_stream_param.u32Profile);
        if (HI_SUCCESS != s32Ret){
            SAMPLE_PRT("Start Venc failed!\n");
        }
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_CHN0);
        if (HI_SUCCESS != s32Ret){
            SAMPLE_PRT("Start Venc failed!\n");
        }
    }
    init_user_vb();
    xop::ChannelPtr channelptr;
    for(i = 0;i < u32ViChnCnt*2; i++){
        int fd = HI_MPI_VENC_GetFd(i);
        if (fd < 0) {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                       fd);
            return NULL;
        }
        channelptr.reset(new xop::Channel(fd));
        channelptr->setReadCallback([this,i](){
            read_stream_cb(i);
        });
        channelptr->enableReading();
        _loop->updateChannel(channelptr);
        _Channels.emplace(i,channelptr);
        channelptr.reset();
    }
    return true;
}

bool Venc::stop_venc() {
    for(auto &item:_Channels) {
        _loop->removeChannel(item.second);
    }
    _Channels.clear();
    int i = 0;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VENC_CHN VencChn;
    for (i = 0; i< 4 ; i++) {
        VencChn = i;
        VpssGrp = i;
        VpssChn = VPSS_CHN0;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
    for (i = 0; i< 4 ; i++) {
        VencChn = i+4;
        VpssGrp = i;
        VpssChn = VPSS_CHN0;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
    return false;
}
void Venc::read_stream_cb(VENC_CHN VencChn) {
    int ret = HI_SUCCESS;
    int len = 0;
    int j = 0;
    VENC_STREAM_S stStream;
    VENC_CHN_STAT_S stStat;
    xop::AVFrame frame;
    memset(&stStream, 0, sizeof(stStream));
    ret = HI_MPI_VENC_Query(VencChn, &stStat);
    if (HI_SUCCESS != ret) {
        SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", VencChn, ret);
        return;
    }
    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
    if (NULL == stStream.pstPack) {
        SAMPLE_PRT("malloc stream pack failed!\n");
        return;
    }
    stStream.u32PackCount = stStat.u32CurPacks;
    ret = HI_MPI_VENC_GetStream(VencChn, &stStream, HI_TRUE);
    if (HI_SUCCESS != ret) {
        free(stStream.pstPack);
        stStream.pstPack = NULL;
        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                               ret);
        return;
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
    if(VencChn < 4 && VencChn >=0) {
        if(VencChn == 2){
            RtspPusherLoop::Instance()->Putframe(frame);
        }
        RecordLoop::Instance()->putframe(VencChn + 1, frame);
        xop::AVFrame my_frame;
        my_frame.buffer.reset(new char[frame.size]);
        if(frame.type == VIDEO_FRAME_I){
            xop::AVFrame sps_frame;
            xop::AVFrame pps_frame;
            sps_frame.buffer.reset(new char[frame.sps_len-4]);
            sps_frame.size = frame.sps_len-4;
            pps_frame.buffer.reset(new char[frame.pps_len-4]);
            pps_frame.size = frame.pps_len-4;
            memcpy(sps_frame.buffer.get(),frame.sps+4,sps_frame.size);
            memcpy(pps_frame.buffer.get(),frame.pps+4,pps_frame.size);
            memcpy(my_frame.buffer.get(),frame.buffer.get()+frame.Iframe_offent+4,frame.Iframe_len-4);
            my_frame.size = frame.Iframe_len-4;
            RtspLoop::get_instance()->putframe(VencChn, sps_frame);
            RtspLoop::get_instance()->putframe(VencChn, pps_frame);
            RtspLoop::get_instance()->putframe(VencChn, my_frame);
        }else{
            my_frame.size = frame.size-4;
            memcpy(my_frame.buffer.get(),frame.buffer.get()+4,my_frame.size);
            RtspLoop::get_instance()->putframe(VencChn, my_frame);
        }
    }
    frame.buffer.reset();
    ret = HI_MPI_VENC_ReleaseStream(VencChn, &stStream);
    if (HI_SUCCESS != ret) {
        free(stStream.pstPack);
        stStream.pstPack = NULL;
        return;
    }
    free(stStream.pstPack);
    stStream.pstPack = NULL;
}
