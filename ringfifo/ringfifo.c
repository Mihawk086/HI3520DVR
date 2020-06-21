#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ringfifo.h"
#include "sample_comm.h"
#define NMAX 16
#define NUM 30
#define GET 1
#define UNGET 0
static int iput = 0; /* 环形缓冲区的当前放入位置 */
static int iget = 0; /* 缓冲区的当前取出位置 */
static int n = 0; /* 环形缓冲区中的元素总数量 */
static int num[NUM] = {0};
static int get[NUM] = {0};
struct ringbuf ringfifo[NMAX];
void ringmalloc(int size)
{
    int i;
    for(i =0; i<NMAX; i++)
    {
        ringfifo[i].buffer = malloc(size);
        ringfifo[i].size = 0;
        ringfifo[i].frame_type = 0;
    }
    iput = 0;
    iget = 0;
    n = 0;
}
void ringreset()
{
    iput = 0; /* 环形缓冲区的当前放入位置 */
    iget = 0; /* 缓冲区的当前取出位置 */
    n = 0; /* 环形缓冲区中的元素总数量 */
}
void  ringreset2(int id)
{
    //iput = 0; /* 环形缓冲区的当前放入位置 */
    get[id] = iget; /* 缓冲区的当前取出位置 */
    num[id] = n; /* 环形缓冲区中的元素总数量 */
}
/**************************************************************************************************
**
**
**

void ringfree(void)
{
    int i;
    printf("begin free mem\n");
    for(i =0; i<NMAX; i++)
    {
       // printf("FREE FIFO INFO:idx:%d,len:%d,ptr:%x\n",i,ringfifo[i].size,(int)(ringfifo[i].buffer));
        free(ringfifo[i].buffer);
        ringfifo[i].size = 0;
    }
}
/**************************************************************************************************
**
**
**
**************************************************************************************************/
int addring(int i)
{
    return (i+1) == NMAX ? 0 : i+1;
}

/**************************************************************************************************
**
**
**
**************************************************************************************************/
/* 从环形缓冲区中取一个元素 */

int ringget(struct ringbuf *getinfo)
{
    int Pos;
    if(n>0)
    {
        Pos = iget;
        iget = addring(iget);
        n--;
        getinfo->buffer = (ringfifo[Pos].buffer);
        getinfo->frame_type = ringfifo[Pos].frame_type;
        getinfo->size = ringfifo[Pos].size;
        //printf("Get FIFO INFO:idx:%d,len:%d,ptr:%x,type:%d\n",Pos,getinfo->size,(int)(getinfo->buffer),getinfo->frame_type);
        return ringfifo[Pos].size;
    }
    else
    {
        //printf("Buffer is empty\n");
        return 0;
    }
}

/**************************************************************************************************
**
**
**
**************************************************************************************************/
/* 从环形缓冲区中取一个元素 */

int ringget2(struct ringbuf *getinfo,int id)
{
    int Pos;
    if((ringfifo[get[id]].getflag == UNGET) && (n>0))
	{
		n--;
        ringfifo[get[id]].getflag = GET;
		iget = addring(iget);
	}
    if(num[id]>0)
    {
        Pos = get[id];
        get[id] = addring(get[id]);
        num[id]--;
        getinfo->buffer = (ringfifo[Pos].buffer);
        getinfo->frame_type = ringfifo[Pos].frame_type;
        getinfo->size = ringfifo[Pos].size;
        //printf("Get FIFO INFO:idx:%d,len:%d,ptr:%x,type:%d\n",Pos,getinfo->size,(int)(getinfo->buffer),getinfo->frame_type);
        return ringfifo[Pos].size;
    }
    else
    {
        //printf("Buffer is empty\n");
        return 0;
    }
}

int ringget3(struct ringbuf *getinfo,int id)
{

}

/**************************************************************************************************
**
**
**
**************************************************************************************************/
/* 向环形缓冲区中放入一个元素*/
void ringput(unsigned char *buffer,int size,int encode_type)
{

    if(n<NMAX)
    {
        memcpy(ringfifo[iput].buffer,buffer,size);
        ringfifo[iput].size= size;
        ringfifo[iput].frame_type = encode_type;
        //printf("Put FIFO INFO:idx:%d,len:%d,ptr:%x,type:%d\n",iput,ringfifo[iput].size,(int)(ringfifo[iput].buffer),ringfifo[iput].frame_type);
        iput = addring(iput);
        n++;
    }
    else
    {
        //  printf("Buffer is full\n");
    }
}

/**************************************************************************************************
**
**
**
**************************************************************************************************/
HI_S32 HisiPutH264DataToBuffer(VENC_STREAM_S *pstStream)
{
	HI_S32 i,j;
	HI_S32 len = 0, off = 0;
	unsigned char *pstr;
	        /*
    if(n == NMAX)
    {
        printf("ringfifo is full \n");
    }
	         */
    if(n < NMAX)
    {
        for (i = 0; i < pstStream->u32PackCount; i++)
        {
            len+=(pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
        }
		for (i = 0; i < pstStream->u32PackCount; i++)
		{
			memcpy(ringfifo[iput].buffer+off,pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
				 pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
			off +=  pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
            pstr = pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
            if(pstr[4] == 0x67)
            {
                printf("find sps\n");
            }
            if(pstr[4] == 0x68)
            {
                printf("find pps\n");
            }
		}

        ringfifo[iput].size= len;

		if(pstStream->stH264Info.enRefType == BASE_IDRSLICE)
		{
            ringfifo[iput].frame_type = 1;
		}
		else
		{
            ringfifo[iput].frame_type = 0;
		}

        iput = addring(iput);
        n++;
		int flag;
		for(flag = 0;flag<NUM ; flag++)
		{
			num[flag]++;
		}
		ringfifo[iput].getflag = UNGET;
        ringfifo[iput].pts = pstStream->pstPack->u64PTS;
    }

	return HI_SUCCESS;
}
