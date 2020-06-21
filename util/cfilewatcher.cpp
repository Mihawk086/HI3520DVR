#include "cfilewatcher.h"
#include "cworkdirectory.h"
#include "cdevstorage.h"
CFileWatcher* CFileWatcher::_instance = new CFileWatcher();
CFileWatcher* CFileWatcher::get_instance()
{
    return _instance;
}
CFileWatcher::CFileWatcher(){
}

int CFileWatcher::Start()
{
    m_configwatch = new CInotifythread(CConfig::get_instance()->getConfigFilePath(),\
                                       CFileWatcher::StartConfigOnInotify,this);
    m_configwatch->start();
}
void* CFileWatcher::onConfigInotify()
{
    cout<<"config file modified"<<endl;
}
void* CFileWatcher::StartConfigOnInotify(void* para)
{
    CFileWatcher *ptr = static_cast<CFileWatcher *>(para);
    ptr->onConfigInotify();
}

