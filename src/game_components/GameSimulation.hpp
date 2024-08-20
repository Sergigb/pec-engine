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

#define VIEW_SIMULATION 1
#define VIEW_PLANETARIUM 2

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

        std::unique_ptr<SimulationRenderer> m_renderer;

        int m_current_view;
        struct thread_monitor* m_thread_monitor;
        bool m_quit;

        void logic();
        void processKeyboardInputSimulation();
        void processKeyboardInputPlanetarium();
        void processInputSimulation();
        void processInput();
        void onRightMouseButton();
        void editorToSimulation();
        void initLaunchBase();
        void setPlayerTarget();
        void switchVessel();
        void updateCameraSimulation();
        void updateCameraPlanetarium();
        void updateSimulation();

        void synchPostStep();
        void synchPreStep();
        void wakePhysics();
        void waitPhysics();
    public:
        GameSimulation(BaseApp* app, const FontAtlas* font_atlas);
        ~GameSimulation();

        int start();

        void setUpSimulation();
};


#endif
