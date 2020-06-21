#include <sys/stat.h>
#include "Mp4Recorder.h"
#include "Util/File.h"
static unsigned char avcCBytes[512]={0};
static FILE* fp_write = NULL;
static int write_buffer(void *opaque, uint8_t *buf, int buf_size){
    //printf("write\n");
    if(!feof(fp_write)){
        int true_size=fwrite(buf,1,buf_size,fp_write);
        fflush(fp_write);
        return true_size;
    }else{
        return -1;
    }
}
static uint32_t getTimeStamp()
{
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	uint32_t ts = ((tv.tv_sec*1000)+((tv.tv_usec+500)/1000)); // 90: _clockRate/1000;
	return ts;
}
static uint32_t GetTickCount()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
        return 0;
    return (tv.tv_sec*1000  + tv.tv_usec / 1000);
}
static  std::string timeStr(const char *fmt) {
    std::tm tm_snapshot;
    auto time = ::time(NULL);
#if defined(_WIN32)
    localtime_s(&tm_snapshot, &time); // thread-safe
#else
    localtime_r(&time, &tm_snapshot); // POSIX
#endif
    const size_t size = 1024;
    char buffer[size];
    auto success = std::strftime(buffer, size, fmt, &tm_snapshot);
    if (0 == success)
        return string(fmt);
    return buffer;
}
Mp4Recorder::Mp4Recorder()
        : m_recordSec(60),
          m_pcontext(nullptr),
          m_pstream(nullptr),
          m_filename(""),
          m_bRecord(true),
          m_nWidth(0),
          m_nHeight(0),
          m_nFrameRate(0),
          m_FirstVideoTime(0),
          m_LastVideoTime(0)
{
    av_register_all();
    memset(&m_avcCBox, 0, sizeof(m_avcCBox));
    memset(m_avcCBox.ppsBuffer, 0, sizeof(m_avcCBox.ppsBuffer));
    memset(m_avcCBox.spsBuffer, 0, sizeof(m_avcCBox.spsBuffer));

}
bool Mp4Recorder::putframe(const xop::AVFrame &frame) {
    if(m_bRecord == true) {
        if (frame.type == VIDEO_FRAME_I && (m_pcontext == NULL || m_ticker.elapsedTime() > m_recordSec * 1000)) {
            memset(&m_avcCBox, 0, sizeof(m_avcCBox));
            memset(m_avcCBox.ppsBuffer, 0, sizeof(m_avcCBox.ppsBuffer));
            memset(m_avcCBox.spsBuffer, 0, sizeof(m_avcCBox.spsBuffer));
            memcpy(m_avcCBox.spsBuffer, frame.sps, frame.sps_len);
            m_avcCBox.sps_length = frame.sps_len;
            memcpy(m_avcCBox.ppsBuffer, frame.pps, frame.pps_len);
            m_avcCBox.pps_length = frame.pps_len;
            closefile();
            createfile();
        }
        if (m_pcontext != NULL) {
            _putframe(frame);
        }
    }
    return true;
}
bool Mp4Recorder::createfile() {
    m_FirstVideoTime = 0;
    m_LastVideoTime = 0;
    m_ticker.resetTime();
    m_pcontext = avformat_alloc_context();
    if(m_pcontext!= NULL)
    {
    }else{
        cout<<"context error"<<endl;
    }
    m_pcontext->oformat = av_guess_format("flv",NULL,NULL);
    if(m_pcontext->oformat == NULL){
        printf("oformat error\n");
        return false;
    }
    m_pstream = avformat_new_stream(m_pcontext, NULL);
    {
        AVCodecContext *c;
        c = m_pstream->codec;
        c->bit_rate = 400000;
        c->codec_id =AV_CODEC_ID_H264;
        c->codec_type = AVMEDIA_TYPE_VIDEO;
        m_pstream->time_base.num = 1;
        m_pstream->time_base.den = 90000;
        c->width = 1280;
        c->height = 720;
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    auto strDate = timeStr("%Y-%m-%d");
    auto strTime = timeStr("%H-%M-%S");
    auto strDir = "./"+strDate;
    auto strFile = strDir + "/" + strDate + "_" + strTime + ".flv";
    m_filename = strFile;
    ZL::Util::File::createfile_path(m_filename.data(), S_IRWXO | S_IRWXG | S_IRWXU);
    m_fp = ZL::Util::File::createfile_file(m_filename.c_str(),"wb+");
    fp_write = m_fp;
    unsigned char* iobuffer = (unsigned char *)av_malloc(32768);
    AVIOContext *avio =avio_alloc_context(iobuffer, 32768,0,NULL,NULL,&write_buffer,NULL);
    m_pcontext->pb=avio;
    m_pcontext->flags = AVFMT_FLAG_CUSTOM_IO;
    avformat_write_header(m_pcontext, NULL);
    return true;
}
bool Mp4Recorder::closefile() {
    if(m_pcontext != NULL) {
        av_write_trailer(m_pcontext);
        av_freep(&m_pcontext->pb->buffer);
        av_freep(&m_pcontext->streams[0]);
        avio_close(m_pcontext->pb);
        av_free(m_pcontext);
        m_pstream = NULL;
        m_pcontext = NULL;
        fclose(m_fp);
        m_fp = NULL;
        fp_write = NULL;
    }
    return false;
}
bool Mp4Recorder::_putframe(const xop::AVFrame &frame) {
    unsigned long long  pts = 0,pts2 = 0, interval = 0;
    AVPacket packet;
    av_init_packet(&packet);
    packet.size = frame.size;
    packet.data = (uint8_t *)frame.buffer.get();
    pts = frame.timestamp;
    packet.flags |=(frame.type == VIDEO_FRAME_I)?AV_PKT_FLAG_KEY:0;
    packet.stream_index = m_pstream->index;
    AVRational time_base;
    time_base.num = 1;
    time_base.den = 1000000;
    if (m_FirstVideoTime == 0){
        m_FirstVideoTime = pts;
    }
    if(m_LastVideoTime == 0){
        m_LastVideoTime = pts;
    }
    pts2 = pts - m_FirstVideoTime;
    interval = pts - m_LastVideoTime;
    if(interval > 80000){
        printf("loss frame\n");
    }
    m_LastVideoTime = pts;
    int curPts = av_rescale_q(pts2, time_base, m_pstream->time_base);
    packet.pts = curPts;
    packet.dts = curPts;
    av_interleaved_write_frame(m_pcontext, &packet);
    av_packet_unref(&packet);
    return false;
}
bool Mp4Recorder::startrecord() {
    m_bRecord = true;
    m_ticker.resetTime();
    return false;
}
bool Mp4Recorder::stoprecord() {
    m_bRecord = false;
    closefile();
    return false;
}
