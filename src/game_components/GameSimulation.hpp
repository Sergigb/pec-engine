#ifndef GAME_SIMULATION_HPP
#define GAME_SIMULATION_HPP

#include <cstdint>
#include <chrono>

class FontAtlas;
class BaseApp;
class SimulationRenderer;
class Physics;
class Player;
class Camera;
class RenderContext;
class AssetManager;
class Input;
class WindowHandler;
class Frustum;

struct thread_monitor;


class GameSimulation{
    private:
        BaseApp* m_app;
        Physics* m_physics;
        Player* m_player;
        Camera* m_camera;
        RenderContext* m_render_context;
        AssetManager* m_asset_manager;
        Input* m_input;
        WindowHandler* m_window_handler;
        Frustum* m_frustum;

        std::chrono::duration<double, std::micro> m_elapsed_time;

        std::unique_ptr<SimulationRenderer> m_renderer;

        void logic();
        void processKeyboardInput();
        void processInput();
        void onRightMouseButton();
        void editorToSimulation();
        void initLaunchBase();
        void setPlayerTarget();
        void switchVessel();
    public:
        GameSimulation(BaseApp* app, const FontAtlas* font_atlas);
        ~GameSimulation();

        void onStateChange();
        void update();
        void updateCamera();
};


#endif
