#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include "memory.h"
#include "Hotplug.h"
#include <iostream>
#include <string>
using namespace std;
Hotplug::Hotplug(xop::EventLoop *eventLoop) :m_eventLoop(eventLoop),m_readBuf(new xop::BufferReader){
    struct sockaddr_nl client;
    struct timeval tv;
    int fd;
    int buffersize = 1024;
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    memset(&client, 0, sizeof(client));
    client.nl_family = AF_NETLINK;
    client.nl_pid = getpid();
    client.nl_groups = 1;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
    bind(fd, (struct sockaddr*)&client, sizeof(client));
    m_hotplugChannel.reset(new xop::Channel(fd));
    m_hotplugChannel->setReadCallback([this](){this->handlRead();});
    m_hotplugChannel->enableReading();
    m_eventLoop->updateChannel(m_hotplugChannel);
}
void Hotplug::setAddDeviceCallback(const AddDeviceCallback &cb) {
    m_addcb = cb;
}
void Hotplug::setRemoveDeviceCallback(const RemoveDeviceCallback &cb) {
    m_removecb = cb;
}
void Hotplug::handlRead() {
    string data;
    int ret = m_readBuf->readFd(m_hotplugChannel->fd());
    if(ret > 0){
        size_t size = 0;
        size = m_readBuf->readAll(data);
        auto found_add = data.find("add");
        auto found_DEVTYPE = data.find("DEVTYPE=disk");
        if(found_add != std::string::npos && found_DEVTYPE != std::string::npos){
            if(m_addcb){
                m_addcb();
            }
        }
        auto found_remove = data.find("remove");
        {
            if(found_remove != std::string::npos && found_DEVTYPE != std::string::npos) {
                if(m_removecb){
                    m_removecb();
                }
            }
        }
    }
    else if(ret <= 0){
    }
}
