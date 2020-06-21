#ifndef HISI3520_CRINGFIFOINSTANCE_H
#define HISI3520_CRINGFIFOINSTANCE_H
#include "CRingFifo.h"
#include "sample_comm.h"
#define CHANNAL_NUM 4
enum BUFFERTYPE
{
    RECORD_BUFFER,
    NET_BUFFER
};
class CRingFifoInstance {
public:
    static CRingFifoInstance* get_instance();
    static CRingFifoInstance* _instance;
    int Init();
    int PutStreamtoBuffer(int channal,int buffertype,VENC_STREAM_S *pstStream);
    int GetSream(int channal,int buffertype,struct ringbuffer *get_buff);
    int ResetStream(int channal,int buffertype);
private:
    CRingFifoInstance();
    CRingFifo* m_RecordFifo[CHANNAL_NUM];
    CRingFifo* m_NetFifo[CHANNAL_NUM];
};
#endif //HISI3520_CRINGFIFOINSTANCE_H
