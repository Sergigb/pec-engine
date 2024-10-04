#ifndef TEXT2D_HPP
#define TEXT2D_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "../core/common.hpp"

class RenderContext;
class FontAtlas;


#define STRING_MAX_LEN 256

// string placement
#define STRING_DRAW_ABSOLUTE_BL 1
#define STRING_DRAW_ABSOLUTE_TL 2
#define STRING_DRAW_ABSOLUTE_TR 3
#define STRING_DRAW_ABSOLUTE_BR 4
#define STRING_DRAW_RELATIVE 5

// string alignment
#define STRING_ALIGN_LEFT 1
#define STRING_ALIGN_CENTER_X 2
#define STRING_ALIGN_CENTER_Y 3
#define STRING_ALIGN_CENTER_XY 4
#define STRING_ALIGN_RIGHT 5 // this should be used when the text is drawn relative to the bottom left/top right


/* used to draw static 2D text. Text should be static (static as in "it's not going to change
 * every few frames") because I create big ass buffers with all the text vertexes/texture
 * coords/indices, and use GL_STATIC_DRAW. I will probably make a new class to draw dynamic text
 * (text that is supposed to change every frame or every few frames) using GL_DYNAMIC_DRAW or
 * GL_STREAM_DRAW. The buffers will be fixed sized and will be flushed when they are full or ready
 * to be drawn. Something like in 
 * https://www.reddit.com/r/opengl/comments/6d4eai/how_do_you_manage_your_quads_when_doing_text/ */
class Text2D{
    private:
        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_col, m_vbo_ind;
        GLuint m_disp_location;
        GLuint m_num_vertices, m_num_indices;
        struct color m_text_color;
        std::vector<struct string> m_strings;
        bool m_update_buffer, m_init;
        int m_fb_width, m_fb_height;
        math::vec2 m_disp;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;

        void updateBuffers();
        void initgl();
        void getPenXY(float& pen_x, float& pen_y, struct string* string_);
    public:
        Text2D();
        /*
         * Constructor.
         * 
         * @fb_width: width of the framebuffer.
         * @fb_height: height of the framebuffer.
         * @font: pointer to the font atlas object, which holds the texture.
         * @render_context: pointer to the app's render context object.
         */
        Text2D(int fb_width, int fb_height, const FontAtlas* font, 
               const RenderContext* render_context);
        ~Text2D();

        /*
         * Adds a string to render.
         * 
         * @string: text to render.
         * @x: absolute x position in the framebuffer.
         * @y: absolute y position in the framebuffer.
         * @scale: scale of the text (1.0 is regular scale).
         * @placement: relative origin of the string, check the macro STRING_DRAW_*.
         * @aligment: aligment of the text, check the macro STRING_ALIGN*.
         * @color: color of the string, includes alpha.
         */
        void addString(const wchar_t* string, uint x, uint y, float scale, 
                       int placement, int alignment, const float color[4]);

        /*
         * Adds a string to render.
         * 
         * @string: text to render.
         * @x: relative x position (0.0 to 1.0) in the framebuffer.
         * @y: relative y position (0.0 to 1.0) in the framebuffer.
         * @scale: scale of the text (1.0 is regular scale).
         * @placement: relative origin of the string, check the macro STRING_DRAW_*.
         * @aligment: aligment of the text, check the macro STRING_ALIGN*.
         * @color: color of the string, includes alpha.
         */        
        void addString(const wchar_t* string, float relative_x, float relative_y,
                       float scale, int alignment, const float color[4]);

        /*
         * Sets the relative displacement of ALL the text contained in this object, the default is 
         * (0.0, 0.0).
         * 
         * @disp: relative displacement in x and y.
         */
        void setDisplacement(const math::vec2& disp);

        /*
         * Clears all the strings.
         */
        void clearStrings();

        /*
         * Returns the height of the font (vertical shift between lines of text).
         */
        uint getFontHeigth() const;

        /*
         * Should be called when the size of the framebuffer changes.
         * 
         * @fb_width: width of the framebuffer.
         * @fb_height: height of the framebuffer.
         */
        void onFramebufferSizeUpdate(int fb_width, int fb_height);

        /*
         * Renders all the strings.
         */
        void render();
};

struct string{
    int posx;
    int posy;
    uint strlen;  // string len without the \n
    short placement;
    short alignment;
    float relative_x;
    float relative_y;
    float scale;
    uint width;
    uint height;
    wchar_t textbuffer[STRING_MAX_LEN];
    float color[4];
};


#endif
