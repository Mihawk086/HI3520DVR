#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include "chotplug.h"
#include "cworkdirectory.h"
#include "cdevstorage.h"
using namespace std;
CHotPlug* CHotPlug::_instance = new CHotPlug();
CHotPlug* CHotPlug::getInstance()
{
    return _instance;
}

CHotPlug::CHotPlug()
{
    m_bAddThread = false;
}


bool CHotPlug::Config(void* (*callback)(void* arg),void *ptr)
{
    _callback  = callback;
    _ptr = ptr;
}


int CHotPlug::RemoveAction()
{
    CDevStorage* pCDevStorage = CDevStorage::get_instance();
    pCDevStorage->UnMountDirectory();
    CWorkDirectory* pCworkDirectory = CWorkDirectory::get_Instance();
    pCworkDirectory->Stop();
}

void* CHotPlug::StartAddAction(void* p)
{
    CHotPlug* ptr = static_cast<CHotPlug*>(p);
    ptr->onAddActionThread();
}

int CHotPlug::AddAction()
{
    pthread_t pid;
    if(m_bAddThread == false)
    {
        m_bAddThread = true;
        pthread_create(&pid,0,CHotPlug::StartAddAction,static_cast<void *>(this));
    }
    return 0;
}

int CHotPlug::onAddActionThread()
{
    CDevStorage* pCDevStorage = CDevStorage::get_instance();
    CWorkDirectory* pCworkDirectory = CWorkDirectory::get_Instance();
    int ret = -1;
    sleep(3);
    ret = pCDevStorage->CheckDevice();
    if(ret == 0)
    {
        pCworkDirectory->Start();
    }
    m_bAddThread = false;
    return 0;
}


void* CHotPlug::run()
{
    struct sockaddr_nl client;
    struct timeval tv;
    int fd, rcvlen, ret;
    fd_set fds;
    int buffersize = 1024;
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    memset(&client, 0, sizeof(client));
    client.nl_family = AF_NETLINK;
    client.nl_pid = getpid();
    client.nl_groups = 1; /* receive broadcast message*/
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
    bind(fd, (struct sockaddr*)&client, sizeof(client));
    while (1)
    {
        char buf[UEVENT_BUFFER_SIZE] = { 0 };
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;
        ret = select(fd + 1, &fds, NULL, NULL, &tv);
        if(ret < 0)
            continue;
        if(!(ret > 0 && FD_ISSET(fd, &fds)))
            continue;
        rcvlen = recv(fd, &buf, sizeof(buf), 0);
        if (rcvlen > 0)
        {
            if(strncmp(buf,"add",3) == 0)
            {
                AddAction();
            }
            else if(strncmp(buf,"remove",6) == 0)
            {
                RemoveAction();
            }
        }
    }
    close(fd);
}
