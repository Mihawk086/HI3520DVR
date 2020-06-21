#ifndef HISI3520_MP4RECORDER_H
#define HISI3520_MP4RECORDER_H
#include "media.h"
#include "Util/TimeTicker.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}
#include <sys/time.h>
class Mp4Recorder {
    typedef struct _MP4AVCC_Box
    {
        int     sps_length;
        int     pps_length;
        unsigned char spsBuffer[256];
        unsigned char ppsBuffer[128];
    }MP4_RecordAvccBox;
public:
    Mp4Recorder();
    bool putframe(const xop::AVFrame &frame);
    bool createfile();
    bool closefile();
    bool startrecord();
    bool stoprecord();
private:
    FILE* m_fp = NULL;
    bool _putframe(const xop::AVFrame &frame);
    ZL::Util::Ticker m_ticker;
    std::string m_filename;
    AVFormatContext* m_pcontext;
    AVStream* m_pstream;
    bool m_bRecord;                //录像状态
    int m_nWidth;
    int m_nHeight;
    int m_nFrameRate;
    unsigned long long  m_FirstVideoTime = 0;
    unsigned long long  m_LastVideoTime = 0;
    int m_recordSec;
    MP4_RecordAvccBox m_avcCBox;
};
#endif //HISI3520_MP4RECORDER_H
