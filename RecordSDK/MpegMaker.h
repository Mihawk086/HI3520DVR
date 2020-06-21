#ifndef HISI3520_MPEGMAKER_H
#define HISI3520_MPEGMAKER_H
#include "mpeg-ps.h"
#include "mpeg-ts.h"
#include "mpeg-ts-proto.h"
#include "media.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <map>
class MpegMaker {
public:
    MpegMaker();
    bool create(std::string filename);
    bool close();
    bool putframe(const xop::AVFrame &frame);
private:
    FILE* m_fp;
    struct mpeg_ts_func_t m_tshandler;
    void* m_ts;
    int m_vediopid;
};
#endif //HISI3520_MPEGMAKER_H
