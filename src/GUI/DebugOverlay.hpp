#ifndef DEBUGOVERLAY_HPP
#define DEBUGOVERLAY_HPP

#include <memory>


class RenderContext;
class Text2D;
class FontAtlas;

struct logic_timing;


class DebugOverlay{
    private:
        std::unique_ptr<Text2D> m_text_debug;
        std::unique_ptr<Text2D> m_text_dynamic_text;
        std::unique_ptr<FontAtlas> m_font_atlas;
        int m_rendered_obj;        
        double m_physics_load_time, m_logic_load_time, m_logic_sleep_time;
        double m_render_load_time, m_rscene_load_time, m_rgui_load_time, m_rimgui_load_time;
    public:
        DebugOverlay(int fb_width, int fb_height, const RenderContext* render_context);
        ~DebugOverlay();

        void setRenderedObjects(int n);
        void setLogicTimes(const logic_timing& times);
        void setRenderTimes(double render_load_time, double rscene_load_time, double rgui_load_time, double rimgui_load_time);
        void onFramebufferSizeUpdate(int fb_width, int fb_height);

        void render();
};

void debug_info_box(Text2D** t, int fb_width, int fb_height, const FontAtlas* font_atlas, const RenderContext* render_context);

#endif