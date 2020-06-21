#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */
#ifndef _VENC_MANAGER_H
#define _VENC_MANAGER_H
#include "sample_comm.h"
typedef struct T_VencConfig
{
	VIDEO_NORM_E gs_enNorm;	
	PIC_SIZE_E enSize[2];
	SAMPLE_RC_E enRcMode;
}T_VencConfig;
int SendVencEvent(T_VencConfig t_VencConfig);
int VencInit();
int VencExit();
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

