#ifndef HISI3520_GUI_MANAGER_H
#define HISI3520_GUI_MANAGER_H
#include "EventLoop.h"
class Gui_Manager {
public:
    Gui_Manager() = delete;
    Gui_Manager(xop::EventLoop* loop);
    bool Init();
    bool Exit();
private:
    xop::EventLoop* _loop;
    int _fd;
};
#endif //HISI3520_GUI_MANAGER_H
