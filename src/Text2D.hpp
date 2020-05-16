#ifndef TEXT2D_HPP
#define TEXT2D_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <cstring>
#include <memory>

#include "common.hpp"
#include "gl_utils.hpp"
#include "maths_funcs.hpp"
#include "FontAtlas.hpp"
#include "utils.hpp"

#define STRING_MAX_LEN 256
#define STRING_DRAW_ABSOLUTE_BL 1
#define STRING_DRAW_ABSOLUTE_TL 2
#define STRING_DRAW_ABSOLUTE_TR 3
#define STRING_DRAW_ABSOLUTE_BR 4
#define STRING_DRAW_RELATIVE 5 // not implemented
#define STRING_ALIGN_LEFT 1 // TODO
#define STRING_ALIGN_CENTER 2
#define STRING_ALIGN_RIGHT 3

class Text2D{
    // used to draw static 2D text. Text should be static (static as in "it's not going to change every few frames") because 
    // I craete big ass buffers with all the text vertexes/texture coords/indices, and use GL_STATIC_DRAW. I will probably
    // make a new class to draw dynamic text (text that is supposed to change every frame or every few frames) using
    // GL_DYNAMIC_DRAW or GL_STREAM_DRAW. The buffers will be fixed sized and will be flushed when they are full or ready
    // to be drawn. Something like in https://www.reddit.com/r/opengl/comments/6d4eai/how_do_you_manage_your_quads_when_doing_text/
    private:
        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind;
        GLuint m_shader_programme, m_texture_id, m_color_location;
        GLuint m_num_vertices, m_num_indices;
        math::mat4 m_projection;
        struct color m_text_color;
        std::vector<struct string> m_strings;
        bool m_update_buffer;
        int m_fb_width, m_fb_height;

        std::unique_ptr<FontAtlas> m_font;

        void updateBuffers();
        void initgl(const color& c);
    public:
        Text2D();
        Text2D(int fb_width, int fb_height, const color& c, uint atlas_size, const char* font_path, int font_size, GLuint shader);
        ~Text2D();

        void addString(const wchar_t* string, uint x, uint y, float scale, int placement);
        void clearStrings();

        void onFramebufferSizeUpdate(int fb_width, int fb_height);
        void render();
};

struct string{
    uint posx;
    uint posy;
    uint strlen;  // string len without the \n
    uint placement;
    float scale;
    wchar_t textbuffer[STRING_MAX_LEN];
};


#endif