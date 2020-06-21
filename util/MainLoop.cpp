#include "MainLoop.h"
#include "RecordLoop.h"
MainLoop& MainLoop::Instance() {
    static MainLoop* instance(new MainLoop());
    return *instance;
}
bool MainLoop::runloop(){
    m_eventloopptr->loop();
    return true;
}
MainLoop::MainLoop():m_eventloopptr(new xop::EventLoop()) {
    m_config_ptr.reset(new Config(m_eventloopptr.get()));
    m_osd_ptr.reset(new Osd(m_eventloopptr.get()));
    m_hisi_ptr.reset(new Hisi_Init(m_eventloopptr.get()));
    m_venc_ptr.reset(new Venc(m_eventloopptr.get()));
    m_gui_ptr.reset(new Gui_Manager(m_eventloopptr.get()));
    m_hisi_ptr->vb_conf();
    m_hisi_ptr->VIO_Init();
    m_osd_ptr->time_osd_init();
    m_osd_ptr->start_time_osd();
    m_venc_ptr->start_venc();
    m_gui_ptr->Init();
    m_config_ptr->setConfigChangeCB_2([this](){
        reload_config();
    });
}
bool MainLoop::reload_config() {
    m_config_ptr->reload_inifile();
    m_config_ptr->get_ini_ptr()->print();
    return true;
}
bool MainLoop::stop() {
    m_eventloopptr->addTriggerEvent([this](){
        m_osd_ptr->stop_time_osd();
        m_venc_ptr->stop_venc();
        m_hisi_ptr->Hisi_Exit();
        RecordLoop::_instance->stoprecord();
    });
    return true;
}
bool MainLoop::start() {
    m_eventloopptr->addTriggerEvent([this](){
        m_hisi_ptr->vb_conf();
        m_hisi_ptr->VIO_Init();
        m_osd_ptr->time_osd_init();
        m_osd_ptr->start_time_osd();
        m_venc_ptr->start_venc();
        RecordLoop::_instance->startrecord();
    });
    return true;
}

