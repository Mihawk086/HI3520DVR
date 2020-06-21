#ifndef _RINGFIFO_H
#define _RINGFIFO_H


#include "sample_comm.h"


struct ringbuf {
    unsigned char *buffer;
	int frame_type;
    int size;
	int getflag;
	unsigned long long pts;
};
//enum H264_FRAME_TYPE {FRAME_TYPE_I, FRAME_TYPE_P, FRAME_TYPE_B};

int addring (int i);
int ringget(struct ringbuf *getinfo);
//void ringput(unsigned char *buffer,int size,int encode_type);
void ringfree();
void ringmalloc(int size);
void ringreset();
HI_S32 HisiPutH264DataToBuffer(VENC_STREAM_S *pstStream);
void ringreset2(int i);
int ringget2(struct ringbuf *getinfo,int i);


#endif
