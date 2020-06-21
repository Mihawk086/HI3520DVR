#include <stdlib.h>
#include "cstoragemanager.h"
#include "cfilewatcher.h"
#include "chotplug.h"
#include "cworkdirectory.h"
#include "cdevstorage.h"
CStorageManager* CStorageManager::get_instance()
{
    return pCStorageManager;
}
CStorageManager* CStorageManager::pCStorageManager = new CStorageManager();
CStorageManager::CStorageManager()
{
}
int CStorageManager::Start()
{
    int ret;
    CHotPlug* pCHotPlug = CHotPlug::getInstance();
    CWorkDirectory* pCWorkDirectory = CWorkDirectory::get_Instance();
    CDevStorage* pCDevStorage = CDevStorage::get_instance();
    CFileWatcher* pCFileWatcher = CFileWatcher::get_instance();
    pCHotPlug->start();
    pCFileWatcher->Start();
    ret = pCDevStorage->CheckDevice();
    if(ret == 0)
    {
        pCWorkDirectory->Start();
    }
}
