#ifndef DEBUGOVERLAY_HPP
#define DEBUGOVERLAY_HPP

#include <memory>


class RenderContext;
class Text2D;
class FontAtlas;

struct logic_timing;
struct physics_timing;
struct render_timing;


struct times_physics{
    double load_time, kinematics_load, orbital_load, gravity_load, bullet_load;

    times_physics(){
        load_time = 0.0;
        kinematics_load = 0.0;
        orbital_load = 0.0;
        gravity_load = 0.0;
        bullet_load = 0.0;
    }
};


struct times_logic{
    double load_time, sleep_time;

    times_logic(){
        load_time = 0.0;
        sleep_time = 0.0;
    }
};


struct times_render{
    double load_time, rscene_load_time, rgui_load_time, rimgui_load_time;

    times_render(){
        load_time = 0.0;
        rscene_load_time = 0.0;
        rgui_load_time = 0.0;
        rimgui_load_time = 0.0;
    }
};

/*
 *  Draws the debug overlay, which includes informations such as load times and number of rendered
 *  objects.
 */
class DebugOverlay{
    private:
        std::unique_ptr<Text2D> m_text_debug;
        std::unique_ptr<Text2D> m_text_dynamic_text;
        std::unique_ptr<FontAtlas> m_font_atlas;
        int m_rendered_obj;        
        times_physics m_times_physics;
        times_logic m_times_logic;
        times_render m_times_render;
    public:
        DebugOverlay(int fb_width, int fb_height, const RenderContext* render_context);
        ~DebugOverlay();

        /*
         * Sets the number of rendered objects during the last frame (useful to test culling).
         * 
         * @n: number of rendered objects.
         */
        void setRenderedObjects(int n);
        
        /*
         * Sets the logic thread load times. Check the structs with the timings defined in
         * core/timing.hpp.
         * 
         * @times: struct with the different load times.
         */
        void setLogicTimes(const logic_timing& times);

        /*
         * Sets the physics thread load times. Check the structs with the timings defined in
         * core/timing.hpp.
         * 
         * @times: struct with the different load times.
         */
        void setPhysicsTimes(const physics_timing& times);

        /*
         * Sets the render thread load times. Check the structs with the timings defined in
         * core/timing.hpp.
         * 
         * @times: struct with the different load times.
         */
        void setRenderTimes(const render_timing& times);
        
        /*
         * Should be called when the framebuffer size changes.
         * 
         * @fb_width: width of the framebuffer.
         * @fb_height: height of the framebuffer.
         */
        void onFramebufferSizeUpdate(int fb_width, int fb_height);

        /*
         * Render function, should be called by the render thread.
         */
        void render();
};

void debug_info_box(Text2D** t, int fb_width, int fb_height, const FontAtlas* font_atlas, const RenderContext* render_context);

#endif
