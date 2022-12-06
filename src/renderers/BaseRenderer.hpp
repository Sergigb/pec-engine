#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP


/*Abstract class for renderer objects*/

struct render_buffer;


class BaseRenderer{
    public:
        BaseRenderer(){};
        virtual ~BaseRenderer(){};

        virtual int render(struct render_buffer* rbuf) = 0;
};


#endif
