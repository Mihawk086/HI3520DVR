#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "CRingFifo.h"
CRingFifo::CRingFifo()
{
}
int CRingFifo::addring(int i)
{
    return (i+1) == m_nbuffer_num ? 0 : i+1;
}
int CRingFifo::ring_get(struct ringbuffer *getbuff) {
    int Pos;
    if(m_num > 0)
    {
        Pos = m_iget;
        m_iget = addring(m_iget);
        m_num--;
        getbuff->buffer = (m_ringfifo[Pos].buffer);
        getbuff->frame_type = m_ringfifo[Pos].frame_type;
        getbuff->size = m_ringfifo[Pos].size;
        getbuff->PTS = m_ringfifo[Pos].PTS;
        if(m_ringfifo[Pos].frame_type == 1) {
            memcpy(getbuff->sps,m_ringfifo[Pos].sps,m_ringfifo[Pos].spslen);
            memcpy(getbuff->pps,m_ringfifo[Pos].pps,m_ringfifo[Pos].ppslen);
            getbuff->ppslen = m_ringfifo[Pos].ppslen;
            getbuff->spslen = m_ringfifo[Pos].spslen;
        }
        return m_ringfifo[Pos].size;
    }
    else
    {
        return 0;
    }
}
int CRingFifo::ring_free() {
    int i;
    printf("begin free mem\n");
    for(i =0; i < m_nbuffer_num; i++)
    {
        if(m_ringfifo[i].buffer != NULL)
        {
            free(m_ringfifo[i].buffer);
        }
        m_ringfifo[i].size = 0;
    }
    return 0;
}
int CRingFifo::ring_malloc(int buffer_size, int buffer_num)
{
    if(buffer_size < 0)
    {
        return -1;
    }
    if(buffer_num < 0)
    {
        return -1;
    }
    m_buffer_size = buffer_size;
    m_nbuffer_num = buffer_num;
    m_ringfifo = (struct ringbuffer*)malloc(sizeof(struct ringbuffer)*m_nbuffer_num);
    if(m_ringfifo == NULL)
    {
        printf("m_ringfifo malloc error\n");
    }
    int i;
    for(i = 0; i< m_nbuffer_num; i++)
    {
        m_ringfifo[i].buffer = (unsigned char*)malloc(buffer_size);
        if(m_ringfifo[i].buffer == NULL)
        {
            printf("malloc ringbuffer erro\n");
        }
        m_ringfifo[i].size = 0;
        m_ringfifo[i].frame_type = 0;
    }
    m_iput = 0; /* 环形缓冲区的当前放入位置 */
    m_iget = 0; /* 缓冲区的当前取出位置 */
    m_num = 0; /* 环形缓冲区中的元素总数量 */
    return 0;
}
int CRingFifo::ring_reset() {
    m_iput = 0; /* 环形缓冲区的当前放入位置 */
    m_iget = 0; /* 缓冲区的当前取出位置 */
    m_num = 0; /* 环形缓冲区中的元素总数量 */
    return 0;
}
int CRingFifo::HisiPutH264DataToBuffer(VENC_STREAM_S *pstStream)
{
    int i,j;
    int len = 0, off = 0;
    unsigned char *pstr;
    int llen = 0;
    unsigned char spsbuf[50];
    std::string spsstring;
    if(m_num < m_nbuffer_num)
    {
        for (i = 0; i < pstStream->u32PackCount; i++)
        {
            len += (pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
        }
        if(m_buffer_size < len)
        {
            return -1;
        }
        for (i = 0; i < pstStream->u32PackCount; i++)
        {
            memcpy(m_ringfifo[m_iput].buffer+off,pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
                   pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
            pstr = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
            llen = pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
            off +=  pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
            if(pstr[4] == 0x67)
            {
                if(llen < 50) {
                    memcpy(m_ringfifo[m_iput].sps, pstr, llen);
                    m_ringfifo[m_iput].spslen = llen;
                }
                else {
                    printf("sps error\n");
                }
            }
            if(pstr[4] == 0x68)
            {
                if(llen < 50) {
                    memcpy(m_ringfifo[m_iput].pps, pstr, llen);
                    m_ringfifo[m_iput].ppslen = llen;
                }
                else {
                    printf("pps error\n");
                }
            }
        }
        m_ringfifo[m_iput].size = len;
        if(pstStream->stH264Info.enRefType == BASE_IDRSLICE)
        {
            m_ringfifo[m_iput].frame_type = 1;
        }
        else
        {
            m_ringfifo[m_iput].frame_type = 0;
        }
        m_ringfifo[m_iput].PTS = pstStream->pstPack->u64PTS;
        m_iput = addring(m_iput);
        m_num++;
    }
    return 0;
}
CRingFifo::~CRingFifo()
{
    ring_free();
    free(m_ringfifo);
}
