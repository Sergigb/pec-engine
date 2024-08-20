#ifndef GAME_SIMULATION_HPP
#define GAME_SIMULATION_HPP

#include <cstdint>
#include <chrono>
#include <vector>
#include <stdint.h>

class FontAtlas;
class BaseApp;
class SimulationRenderer;
class PlanetariumRenderer;
class PlanetariumGUI;
class Physics;
class Player;
class Camera;
class RenderContext;
class AssetManager;
class Input;
class WindowHandler;
class Frustum;
class Planet;
class Predictor;

struct thread_monitor;

#define VIEW_SIMULATION 1
#define VIEW_PLANETARIUM 2

#define PLANETARIUM_SCALE_FACTOR 1e9


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
        const Predictor* m_predictor;

        std::unique_ptr<SimulationRenderer> m_renderer_simulation;
        std::unique_ptr<PlanetariumRenderer> m_renderer_planetarium;
        std::unique_ptr<PlanetariumGUI> m_gui_planetarium;

        int m_current_view;
        struct thread_monitor* m_thread_monitor;
        bool m_quit;

        uint32_t m_selected_planet;
        uint m_selected_planet_idx;
        bool m_freecam;
        std::vector<const Planet*> m_ordered_planets;

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
        void switchPlanet();
        void updateCameraSimulation();
        void updateCameraPlanetarium();
        //void updateSimulation();
        void updatePlanetarium();

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
