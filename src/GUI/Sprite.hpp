#ifndef SPRITE_HPP
#define SPRITE_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../core/common.hpp"
#include "../core/maths_funcs.hpp"


// sprite placement
#define SPRITE_DRAW_ABSOLUTE 1
#define SPRITE_DRAW_RELATIVE 2

class RenderContext;


/*
 * This class is used to draw a single sprite on the screen. The placement of the sprite can be 
 * relative to the size of the framebuffer (coords should be between 0 and 1) or absolute (in 
 * framebuffer coordinates).
 */
class Sprite{
    private:
        GLuint m_vao, m_vbo_vert, m_vbo_tex;
        GLuint m_sprite;
        GLuint m_disp_location, m_alpha_location;
        float m_fb_width, m_fb_height;
        math::vec2 m_pos;
        short m_positioning;
        float m_size, m_alpha;
        bool m_free_on_destruction;
        

        const RenderContext* m_render_context;

        void initgl(const char* path);
        void updateVertexArray();
    public:
        Sprite();
        /*
         * Constructor.
         *
         * @render_context: pointer to the RenderContext object.
         * @pos: 2D coordinates of the sprite. Either absolute (between 0 and 1) or relative (in
         * framebuffer coordinates).
         * @positioning: indicates if the coordinates have to be interpreted relative to the 
         * framebuffer size (SPRITE_DRAW_RELATIVE) or absolute (SPRITE_DRAW_ABSOLUTE).
         * @path: path to the image sprite.
         * @size: size of the sprite (in pixels).
         */
        Sprite(const RenderContext* render_context, const math::vec2& pos, short positioning,
               const char* path, const float size);

        // DO NOT USE - READ BELOW (just use pointers or whatever, do not copy)
        Sprite(const Sprite& sprite);

        /* VERY IMPORTANT!!
         * Sprites should only be move-assigned or moved. When these two constructors are called,
         * the m_free_on_destruction variable of the object being moved is set to false, which
         * means that the destructor of the moved object will not free the gl data.
         */
        Sprite(Sprite&& sprite);
        Sprite& operator=(Sprite&& sprite);
        ~Sprite();

        /*
         * Updates the position of the sprite.
         *
         * @pos: new position of the sprite.
         */
        void updatePos(const math::vec2& pos);

        /*
         * Updates the size of the sprite.
         *
         * @size: size of the sprite (in coordinates).
         */
        void updateSize(const float size);

        /*
         * This method should be called if the size of the framebuffer changes.
         */
        void onFramebufferSizeUpdate();

        /*
         * Render method.
         */
        void render() const;

        /*
         * Render method given the position. This is a hacky way to render the sprite when the
         * caller has a constant pointer to the planet. Maybe the sprites should be managed by the
         * PlanetariumGUI.
         *
         * @pos: position of the sprite.
         */
        void render(const math::vec2& pos) const;

        /*
         * Sets the alpha of the sprite (transparency)
         *
         * @alpha: transparency value, 1.0 opaque, 0.0 completely transparent.
         */
        void setAlpha(const float alpha);
};


#endif