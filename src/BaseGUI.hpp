#ifndef BASEGUI_HPP
#define BASEGUI_HPP

#include "WindowHandler.hpp"


/*Abstract class for GUI objects*/


class BaseGUI{
    protected:
    public:

        BaseGUI(){};
        virtual ~BaseGUI(){};

        virtual void onFramebufferSizeUpdate() = 0;
        virtual void render() = 0;
        virtual void update() = 0;
};


#endif
