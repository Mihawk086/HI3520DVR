#include "csaveinterface.h"
HI_S32 RequestIDR(VENC_CHN VeChn)
{
    HI_S32 s32Ret;
    s32Ret = HI_MPI_VENC_RequestIDR(VeChn,HI_TRUE);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_RequestIDR err 0x%xn",s32Ret);
    }
    return s32Ret;
}
CSaveInterface* CSaveInterface::_m_pSaveInterface = new CSaveInterface();
CSaveInterface::CSaveInterface()
{
    pthread_mutex_init(&m_mutex, NULL);
    for(int i = 0 ; i < 4 ; i++)
    {
        m_mediafile[i] = new CMediaFile(i+1);
    }
}

CSaveInterface::~CSaveInterface()
{
    for(int i = 0 ; i < 4 ; i++)
    {
        delete m_mediafile[i];
    }
     pthread_mutex_destroy(&m_mutex);
}

CSaveInterface* CSaveInterface::fnGetInstance()
{
    return _m_pSaveInterface;
}
bool CSaveInterface::WriteH264(int channal,VENC_STREAM_S *pstStream,VENC_CHN VeChn)
{
    int ret;
    ret = pthread_mutex_trylock(&m_mutex);
    if(m_newfile[channal-1] == true)
    {
        if(pstStream->stH264Info.enRefType != BASE_IDRSLICE)
        {
            RequestIDR(VeChn);
            pthread_mutex_unlock(&m_mutex);
            return false;
        }
        else
        {
            m_newfile[channal-1] = false;
        }
    }
    if(ret == 0)
    {
        for (int i = 0; i < pstStream->u32PackCount; i++)
        {
            m_mediafile[channal-1]->NormalWrite(
            pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
            pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
        }
    }
    else
    {
        return false;
    }
    pthread_mutex_unlock(&m_mutex);
    return true;
}
bool CSaveInterface::WriteInterface(int channal,VENC_STREAM_S *pstStream,VENC_CHN VeChn)
{
    if((channal < 1) || (channal > 4))
    {
        return false;
    }
    if(m_mediafile[channal-1]->IsOpened() == false)
    {
        return false;
    }
    WriteH264(channal,pstStream,VeChn);
    return true;
}


bool CSaveInterface::Create(int channal,string& path)
{
    pthread_mutex_lock(&m_mutex);
    m_mediafile[channal-1]->NormalClose();
    m_mediafile[channal-1]->CreateMediaFile(path);
    m_mediafile[channal-1]->NormalOpen();
    pthread_mutex_unlock(&m_mutex);
    return true;
}
bool CSaveInterface::Create(string& path)
{
    pthread_mutex_lock(&m_mutex);
    for(int i = 0; i < 4 ; i++)
    {
        m_mediafile[i]->NormalClose();
        m_mediafile[i]->CreateMediaFile(path);
        m_mediafile[i]->NormalOpen();
        m_newfile[i] = true;
    }
    pthread_mutex_unlock(&m_mutex);
    return true;
}
bool CSaveInterface::CloseAll()
{
    for(int i = 0; i < 4 ; i++)
    {
        m_mediafile[i]->NormalClose();
    }
    return true;
}



