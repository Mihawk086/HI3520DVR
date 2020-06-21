#include "cworkdirectory.h"
CWorkDirectory* CWorkDirectory::m_pWorkDirectory = NULL;
CWorkDirectory* CWorkDirectory::get_Instance()
{
    if(m_pWorkDirectory == NULL)
    {
        m_pWorkDirectory = new CWorkDirectory();
    }
    return m_pWorkDirectory;
}
CWorkDirectory::CWorkDirectory()
{
    pthread_mutex_init(&m_directorymutex, NULL);
    m_WorkDirectory = "/mnt";
    m_sizetofree_mb = 1000;
    m_IsRecording = false;
}
CWorkDirectory::~CWorkDirectory()
{
    pthread_mutex_destroy(&m_directorymutex);
}
bool CWorkDirectory::SetCurrentDirectory(string& Currentpath)
{
    pthread_mutex_lock(&m_directorymutex);
    m_CurrentDirectory = Currentpath;
    pthread_mutex_unlock(&m_directorymutex);
}
string CWorkDirectory::getCurrentDirectory()
{
    return this->m_CurrentDirectory;
}
string CWorkDirectory::getWorkDirectory()
{
    return this->m_WorkDirectory;
}
bool CWorkDirectory::getSpace()
{
    struct statfs diskInfo;
    statfs(m_WorkDirectory.c_str(), &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目
    //unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小
    m_totalsize_mb = totalsize>>20;
    m_availablesize_mb = availableDisk>>20;
    return true;
}
void* CWorkDirectory::OnTimerInfo()
{}
void* CWorkDirectory::OnTimerSpaceCheck()
{
    getSpace();
    while(m_availablesize_mb < m_sizetofree_mb)
    {
        string path,filepath;
        int ret;
        ret = m_sqlitemanager->SearchOldestFile(path,filepath);
        m_sqlitemanager->DeletebyFilepath(filepath);
        cout<<"free space "<<endl;
        cout<<filepath<<endl;
        cout<<""<<endl;
        ret = remove(filepath.c_str());
        if(ret < 0)
        {
            cout<<"remove error"<<endl;
        }
        getSpace();
    }
}
bool CWorkDirectory::CreatNewFolder()
{
    m_currenttime.SetCurrentTime();
    char buf[100];
    sprintf(buf,"%04d_%02d_%02d",m_currenttime.GetNormalTime().year,\
            m_currenttime.GetNormalTime().month,m_currenttime.GetNormalTime().day);
    string directory = buf;
    string CurrentDirectory;
    CurrentDirectory = m_WorkDirectory;
    CurrentDirectory += "/";
    CurrentDirectory += directory;
    if(access(CurrentDirectory.c_str(),F_OK) !=0)
    {
        mkdir(CurrentDirectory.c_str(),0777);
        chmod(CurrentDirectory.c_str(),0777);
        m_CurrentDirectory = CurrentDirectory;
        return true;
    }
    else
    {
        m_CurrentDirectory = CurrentDirectory;
        return false;
    }
}
void* CWorkDirectory::OnTimerLoop()
{
    CreatNewFolder();
    pthread_mutex_lock(&m_directorymutex);
    m_saveinterface->Create(m_CurrentDirectory);
    pthread_mutex_unlock(&m_directorymutex);
}
bool CWorkDirectory::ReadConfig()
{
    int i;
    T_ChannalSetting tChannalSetting[4];
    m_config->ReadJSON();
    T_RecordMenu tRecordMenu = m_config->GetRecordMenu();
    for(i = 0; i < 4;i++)
    {
        tChannalSetting[i] = m_config->GetChannalSetting(i);
    }
    if(tRecordMenu.recordmode == Close)
    {
        m_IsRecording = false;
    }
    else
    {
        for(i = 0; i < 4;i++)
        {
            if(tChannalSetting[i].enable == true)
            {
                m_IsRecording = true;
                break;
            }
        }
    }
    return true;
}
bool CWorkDirectory::Start()
{
    m_saveinterface = CSaveInterface::fnGetInstance();
    m_sqlitemanager = CSQLiteManager::get_instance();
    m_config = CConfig::get_instance();
    ReadConfig();
    m_sqlitemanager->CreateDB();
    m_sqlitemanager->CheckDB();
    m_sqlitemanager->PrintDB();
    if(m_IsRecording == true)
    {
        CreatNewFolder();
        pthread_mutex_lock(&m_directorymutex);
        m_saveinterface->Create(m_CurrentDirectory);
        pthread_mutex_unlock(&m_directorymutex);
    }
}
bool CWorkDirectory::Stop()
{
    m_timer_loopcoverage.stop();
    m_timer_spacecheck.stop();
    m_saveinterface->CloseAll();
}
void CWorkDirectory::StartOnTimerLoop(void* para)
{
    CWorkDirectory *ptr = static_cast<CWorkDirectory *>(para);
    ptr->OnTimerLoop();
}
void CWorkDirectory::StartOnTimerInfo(void* para)
{
    CWorkDirectory *ptr = static_cast<CWorkDirectory *>(para);
    ptr->OnTimerInfo();
}
void CWorkDirectory::StartOnTimerSpaceCheck(void* para)
{
    CWorkDirectory *ptr = static_cast<CWorkDirectory *>(para);
    ptr->OnTimerSpaceCheck();
}
