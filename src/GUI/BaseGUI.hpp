#ifndef BASEGUI_HPP
#define BASEGUI_HPP


/*Abstract class for GUI objects*/


class BaseGUI{
    protected:
    public:

        BaseGUI(){};
        virtual ~BaseGUI(){};

        virtual void onFramebufferSizeUpdate() = 0;
        virtual void render() = 0;
        virtual int update() = 0;
        virtual void renderImGUI() = 0;
};


#endif
