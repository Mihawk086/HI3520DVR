#include "CRingFifoInstance.h"
CRingFifoInstance *CRingFifoInstance::get_instance()
{
    static CRingFifoInstance instance;
    return &instance;
}
int CRingFifoInstance::Init()
{
    int i = 0;
    for(i = 0; i < CHANNAL_NUM ; i++)
    {
        m_NetFifo[i] = new CRingFifo();
        m_NetFifo[i]->ring_malloc(1024*512,10);
    }
    for(i = 0; i < CHANNAL_NUM ; i++)
    {
        m_RecordFifo[i] = new CRingFifo();
        m_RecordFifo[i]->ring_malloc(1024*512,10);
    }
    return 0;
}

int CRingFifoInstance::PutStreamtoBuffer(int channal, int buffertype, VENC_STREAM_S *pstStream)
{
    if(buffertype == 0)
    {
        m_NetFifo[channal]->HisiPutH264DataToBuffer(pstStream);
    }
    if(buffertype == 1)
    {
        m_RecordFifo[channal]->HisiPutH264DataToBuffer(pstStream);
    }
    return 0;
}

int CRingFifoInstance::GetSream(int channal, int buffertype, struct ringbuffer *get_buff) {
    int len = 0;
    if(buffertype == 0)
    {
        len = m_NetFifo[channal]->ring_get(get_buff);
    }
    else if(buffertype == 1)
    {
        len = m_RecordFifo[channal]->ring_get(get_buff);
    }
    return len;
}
CRingFifoInstance::CRingFifoInstance() {
    Init();
}
int CRingFifoInstance::ResetStream(int channal, int buffertype) {
    if(buffertype == 0)
    {
        m_NetFifo[channal]->ring_reset();
    }
    if(buffertype == 1)
    {
        m_RecordFifo[channal]->ring_reset();
    }
    return 0;
}
