#ifndef HISI3520_HOTPLUG_H
#define HISI3520_HOTPLUG_H
#include "Channel.h"
#include "BufferReader.h"
#include "EventLoop.h"
typedef std::function<void()> AddDeviceCallback;
typedef std::function<void()> RemoveDeviceCallback;
class Hotplug {
public:
    Hotplug() = delete;
    Hotplug(xop::EventLoop* eventLoop);
    void setAddDeviceCallback(const AddDeviceCallback& cb);
    void setRemoveDeviceCallback(const RemoveDeviceCallback& cb);
private:
    void handlRead();
    xop::EventLoop* m_eventLoop = nullptr;
    xop::ChannelPtr m_hotplugChannel;
    std::shared_ptr<xop::BufferReader> m_readBuf;
    AddDeviceCallback m_addcb;
    RemoveDeviceCallback m_removecb;
};
#endif //HISI3520_HOTPLUG_H
