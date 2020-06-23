#ifndef BASEGUI_HPP
#define BASEGUI_HPP

#include "WindowHandler.hpp"


/*Abstract class for GUI objects*/


class BaseGUI{
    public:
        const WindowHandler* m_window_handler;

        BaseGUI(){};
        BaseGUI(const WindowHandler* window_handler): m_window_handler(window_handler){};
        virtual ~BaseGUI(){};

        virtual void onFramebufferSizeUpdate() = 0;
        virtual void render() = 0;
        virtual void update() = 0;
};


#endif
