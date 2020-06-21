#include "MpegMaker.h"
static int ts_stream(void* ts, int codecid)
{
    static std::map<int, int> streams;
    std::map<int, int>::const_iterator it = streams.find(codecid);
    if (streams.end() != it)
        return it->second;
    int i = mpeg_ts_add_stream(ts, codecid, NULL, 0);
    streams[codecid] = i;
    return i;
}
static void* ts_alloc(void* /*param*/, size_t bytes)
{
    static char s_buffer[188];
    assert(bytes <= sizeof(s_buffer));
    return s_buffer;
}
static void ts_free(void* /*param*/, void* /*packet*/)
{
    return;
}
static void ts_write(void* param, const void* packet, size_t bytes)
{
    fwrite(packet, bytes, 1, (FILE*)param);
}
MpegMaker::MpegMaker():m_fp(NULL) {
    m_tshandler.alloc = ts_alloc;
    m_tshandler.write = ts_write;
    m_tshandler.free = ts_free;
}
bool MpegMaker::create(std::string filename) {
    m_fp = fopen(filename.c_str(), "wb");
    m_ts = mpeg_ts_create(&m_tshandler, m_fp);
    m_vediopid = mpeg_ts_add_stream(m_ts,PSI_STREAM_H264, NULL, 0);
    return false;
}
bool MpegMaker::close() {
    if(m_ts != NULL) {
        mpeg_ts_destroy(m_ts);
        m_ts = NULL;
    }
    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }
    return false;
}
bool MpegMaker::putframe(const xop::AVFrame &frame) {
    if(m_ts !=NULL && m_fp!=NULL) {
        int64_t pts = frame.timestamp / 1000 * 90;
        int64_t dts = pts;
        int flags = (frame.type == VIDEO_FRAME_I) ? 1 : 0;
        mpeg_ts_write(m_ts, m_vediopid, flags, pts, dts, frame.buffer.get(), frame.size);
    }
    return false;
}
