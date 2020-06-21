#ifndef HISI3520_MAINLOOP_H
#define HISI3520_MAINLOOP_H
#include <memory>

#include "EventLoop.h"
#include "Config.h"
#include "Osd.h"
#include "Venc.h"
#include "Hisi_Init.h"
#include "Gui_Manager.h"
class MainLoop {
public:
    static MainLoop& Instance();
    Config& GetConfig(){return *m_config_ptr;}
    bool stop();
    bool start();
    bool runloop();
    bool reload_config();
private:
    MainLoop();
    shared_ptr<Osd> m_osd_ptr;
    shared_ptr<Config> m_config_ptr;
    shared_ptr<xop::EventLoop> m_eventloopptr;
    shared_ptr<Venc> m_venc_ptr;
    shared_ptr<Hisi_Init> m_hisi_ptr;
    shared_ptr<Gui_Manager> m_gui_ptr;
};
#endif //HISI3520_MAINLOOP_H
