#ifndef PLAYER_HPP
#define PLAYER_HPP


#define PLAYER_BEHAVIOUR_NONE 0x01
#define PLAYER_BEHAVIOUR_EDITOR 0x02
#define PLAYER_BEHAVIOUR_SIMULATION 0x04
#define PLAYER_BEHAVIOUR_PLANETARIUM 0x08

#define ORBITAL_CAM_MODE_SURFACE 0
#define ORBITAL_CAM_MODE_ORBIT 1

class Vessel;
class Camera;
class AssetManager;
class Input;
class BaseApp;


class Player{
    private:
        Vessel* m_vessel; // user controlled vessel
        Camera* m_camera;
        AssetManager* m_asset_manager;
        const Input* m_input;
        const BaseApp* m_app;
        short m_behaviour, m_orbital_cam_mode;
        std::uint32_t m_selected_planet;
        bool m_planetarium_freecam;
        double m_planetarium_scale_factor;

        void togglePlanetariumFreecam();
        void unsetVessel();
        void setCamTarget();
        void switchVessel();
        //void switchPlanet(); see setSelectedPlanet
        void setCamAxisRotation();
        void processInput();
        void updateCamera();
    public:
        Player(Camera* camera, AssetManager* asset_manager, const Input* input);
        ~Player();

        void update();

        // this method should be called by the player controlled vessel
        void onVesselDestroy();
        void setBehaviour(short behaviour);
        // for now, switching planets in the planetarium is controlled by the app
        void setSelectedPlanet(std::uint32_t planet_id);
        void setPlanetariumScaleFactor(double factor);

        Vessel* getVessel() const;
        bool getPlanetariumFreecam() const;
};


#endif
