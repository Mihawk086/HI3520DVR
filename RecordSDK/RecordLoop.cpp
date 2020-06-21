#include <iostream>
#include <pthread.h>
#include <sys/statfs.h>
#include <cstring>
#include <dirent.h>
#include "cdevstorage.h"
#include "RecordLoop.h"
#include "csqlitemanager.h"
RecordLoop* RecordLoop::_instance = nullptr;
static void* RecordLoop_fuction(void* p)
{
    RecordLoop* ptr = static_cast<RecordLoop*>(p);
    ptr->loop();
}
static void* RecordLoop_fuction2(void* p)
{
    RecordLoop* ptr = static_cast<RecordLoop*>(p);
    ptr->loop2();
}
RecordLoop *RecordLoop::Instance()
{
    if(_instance == nullptr){
        _instance = new RecordLoop();
        pthread_t id1,id2;
        pthread_create(&id1, nullptr,RecordLoop_fuction,(void*)(_instance));
        pthread_create(&id2, nullptr,RecordLoop_fuction2,(void*)(_instance));
    }
    return _instance;
}
bool RecordLoop::putframe(const xop::AVFrame &frame) {
    std::lock_guard<std::mutex> locker(_mutex);
    m_eventLoop.addTriggerEvent([this,frame](){
    });
    return false;
}
RecordLoop::RecordLoop():
        m_fileindex_eventLoop(),
        m_eventLoop(),m_IsRecord(true),m_Path("/mnt/"),m_bSDcardReady(false),m_remove_mb(500)
{
    m_MapMediaFile.insert(make_pair(1,std::make_shared<MediaFile>(m_Path,CHANNEL_1)));
    m_MapMediaFile.insert(make_pair(2,std::make_shared<MediaFile>(m_Path,CHANNEL_2)));
    m_MapMediaFile.insert(make_pair(3,std::make_shared<MediaFile>(m_Path,CHANNEL_3)));
    m_MapMediaFile.insert(make_pair(4,std::make_shared<MediaFile>(m_Path,CHANNEL_4)));
    for(int i = 1; i<=4;i++) {
        m_MapMediaFile[i]->setWriteSqilteCB([this](const DVR_TABLE &table){
            write_sqlite(table);});
    }
    if(CDevStorage::get_instance()->CheckDevice() == 0) {
        cout<<"SDcard mount"<<endl;
        CSQLiteManager::get_instance()->CreateDB();
        m_bSDcardReady = true;
        m_IsRecord = true;
    };
    m_fileindex_eventLoop.addTimer([this](){
        if((m_IsRecord == true) && (m_bSDcardReady == true)) {
            string path,filepath;
            int ret = 0;
            int count = 0;
            getSpace();
            while (m_availablesize_mb < m_remove_mb) {
                if(CSQLiteManager::get_instance()->SearchOldestFile(path,filepath)==true) {
                    CSQLiteManager::get_instance()->DeletebyFilepath(filepath);
                    cout << "free space " << endl;
                    cout << filepath << endl;
                    ret = remove(filepath.c_str());
                    if (ret < 0) {
                        cout << "remove error" << endl;
                    }
                }else{
                    cout<<"free space error"<<endl;
                }
                getSpace();
                count++;
                if(count>=10) {
                    cout<<"free space error"<<endl;
                    stoprecord();
                    break;
                }
            }
        }
    },5000,true);
    m_hotplug.reset(new Hotplug(&m_eventLoop));
    m_hotplug->setAddDeviceCallback([this](){
        m_eventLoop.addTimer([this](){
            if(CDevStorage::get_instance()->CheckDevice() == 0) {
                cout<<"SDcard mount"<<endl;
                m_bSDcardReady = true;
                m_IsRecord = true;
                CSQLiteManager::get_instance()->CreateDB();
            } else{
                cout<<"SDcard mount error"<<endl;
            }
            },4000,false);});
    m_hotplug->setRemoveDeviceCallback([this](){
        m_eventLoop.addTriggerEvent([this](){
            CDevStorage::get_instance()->UnMountDirectory();
            m_bSDcardReady = false;
            m_IsRecord = false;
            stoprecord();
        });
    });
}
int RecordLoop::loop() {m_eventLoop.loop();}
int RecordLoop::loop2() {m_fileindex_eventLoop.loop();}
bool RecordLoop::putframe(int channel, const xop::AVFrame &frame) {
    std::lock_guard<std::mutex> locker(_mutex);
    if((m_IsRecord == true) && (m_bSDcardReady == true)) {
        m_eventLoop.addTriggerEvent([this,channel,frame]() {
                    m_MapMediaFile[channel]->PutFrame(frame);
        });
    }
    return false;
}
bool RecordLoop::startrecord() {
    std::lock_guard<std::mutex> locker(_mutex);
    m_eventLoop.addTriggerEvent([this](){
        if(CDevStorage::get_instance()->CheckDevice() == 0) {
            cout<<"SDcard mount"<<endl;
            CSQLiteManager::get_instance()->CreateDB();
            m_bSDcardReady = true;
        };
        if(m_bSDcardReady == true) {
            m_IsRecord = true;
        }
    });
    return true;
}
bool RecordLoop::stoprecord() {
    std::lock_guard<std::mutex> locker(_mutex);
    m_eventLoop.addTriggerEvent([this](){
        m_IsRecord = false;
        for(int i = 1; i<=4;i++) {
            m_MapMediaFile[i]->StopRecord();
        }
    });
    return true;
}
bool RecordLoop::getSpace()
{
    struct statfs diskInfo;
    statfs(m_Path.c_str(), &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小
    m_totalsize_mb = totalsize>>20;
    m_availablesize_mb = availableDisk>>20;
    return true;
}
void RecordLoop::write_sqlite(const DVR_TABLE &table) {
    m_fileindex_eventLoop.addTriggerEvent([table](){
        CSQLiteManager::get_instance()->WriteDB(table);
    });
}


