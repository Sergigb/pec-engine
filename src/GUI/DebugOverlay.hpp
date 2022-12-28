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

        void setRenderedObjects(int n);
        void setLogicTimes(const logic_timing& times);
        void setPhysicsTimes(const physics_timing& times);
        void setRenderTimes(const render_timing& times);
        void onFramebufferSizeUpdate(int fb_width, int fb_height);

        void render();
};

void debug_info_box(Text2D** t, int fb_width, int fb_height, const FontAtlas* font_atlas, const RenderContext* render_context);

#endif