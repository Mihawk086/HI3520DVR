#ifndef HISI3520_CRINGFIFO_H
#define HISI3520_CRINGFIFO_H
#include <string>
#include "sample_comm.h"
#include <vector>
#define BUFFER_NUM 10
struct ringbuffer{
    unsigned char *buffer;
    int frame_type;         //帧类型
    int size;               //大小
    int getflag;            //
    unsigned long long PTS;
    unsigned char sps[50];
    unsigned char pps[50];
    int spslen;
    int ppslen;
};
class CRingFifo {
public:
    CRingFifo();
    ~CRingFifo();
    int addring (int i);
    int ring_get(struct ringbuffer *getbuff);
    int ring_free();
    int ring_malloc(int buffer_size ,int buffer_num);
    int ring_reset();
    int HisiPutH264DataToBuffer(VENC_STREAM_S *pstStream);
private:
    int m_iput = 0;
    int m_iget = 0;
    int m_num = 0;
    struct ringbuffer* m_ringfifo;
    int m_buffer_size = 0;
    int m_nbuffer_num;
};
#endif //HISI3520_CRINGFIFO_H
