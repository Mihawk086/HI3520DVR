#ifndef HISI3520_CONFIG_H
#define HISI3520_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "EventLoop.h"
#include "Channel.h"
#include "inifile.h"
#include "BufferReader.h"
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
typedef std::function<void()> ConfigChangeCB;
class Config {
public:
    Config()= delete;
    Config(xop::EventLoop* loop);
    bool setConfigChangeCB(const ConfigChangeCB& cb);
    int test_read();
    bool setConfigChangeCB_2(const ConfigChangeCB& cb){m_ConfigChangeCB_2 = cb;};
    bool set_socket();
    bool reload_inifile();
    std::shared_ptr<inifile::IniFile> get_ini_ptr(){return m_inifile_ptr;}
private:
    bool handleRead();
    bool handleRead2();
    int m_fd_2;
    int m_fd;
    int m_wd;
    char m_buf[EVENT_BUF_LEN];
    string m_ConfigPath;
    std::shared_ptr<xop::BufferReader> m_readBuf;
    std::shared_ptr<inifile::IniFile> m_inifile_ptr;
    xop::EventLoop* m_loop;
    xop::ChannelPtr m_ConfigChannel;
    ConfigChangeCB m_ConfigChangeCB;
    xop::ChannelPtr m_ConfigChannel_2;
    ConfigChangeCB m_ConfigChangeCB_2;
};
#endif //HISI3520_CONFIG_H
