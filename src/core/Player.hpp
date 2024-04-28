#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Camera.hpp"

#define PLAYER_BEHAVIOUR_NONE 0x01
#define PLAYER_BEHAVIOUR_EDITOR 0x02
#define PLAYER_BEHAVIOUR_SIMULATION 0x04
#define PLAYER_BEHAVIOUR_PLANETARIUM 0x08

/* Camera orbit mode */
#define ORBITAL_CAM_MODE_SURFACE 0
#define ORBITAL_CAM_MODE_ORBIT 1


class Vessel;
class Camera;
class AssetManager;
class Input;
class BaseApp;


/*
 * Player class, has code that can be used in different applications (camera update, for example),
 * also holds information such as the vessel that the player is controlling or the planet selected
 * in the planetarium view.
 */
class Player{
    private:
        Vessel* m_vessel; // user controlled vessel
        Camera* m_camera;
        AssetManager* m_asset_manager;
        const Input* m_input;
        const BaseApp* m_app;
        short m_behaviour, m_orbital_cam_mode;
        std::uint32_t m_selected_planet;
        struct camera_params m_planetarium_cam_params;
        struct camera_params m_simulation_cam_params;
        struct camera_params m_editor_cam_params;

        void togglePlanetariumFreecam();
        void switchVessel();
    public:
        Player(Camera* camera, AssetManager* asset_manager, const Input* input);
        ~Player();

        /*
         * This method should only be called by the player controlled vessel in case it gets
         * destroyed, ignore otherwise.
         */ 
        void onVesselDestroy();

        /*
         * Changes the behaviour of the player, specifically the way the input is processed.
         */
        void setBehaviour(short behaviour);
        
        /*
         * Sets the selected planet in the planetarium view. Switching planets is, for now, 
         * controlled by the application (BaseApp) class.
         *
         * @planet_id: ID of the planet.
         */
        void setSelectedPlanet(std::uint32_t planet_id);

        /*
         * Returns a raw pointer to the user controlled vessel.
         */
        Vessel* getVessel() const;

        /*
         * Returns the behaviour of the player, the possible behaviours of the player are defined
         * at the top of this file (macros starting with PLAYER_BEHAVIOUR_*).
         */
        short getBehaviour() const;

        /*
         * Returns the ID of the selected planet in the planetarium.
         */
        std::uint32_t getPlanetariumSelectedPlanet() const;

        /*
         * Unsets the player vessel and notifies the controlled vessel.
         */
        void unsetVessel();

        /*
         * Sets the player controlled vessel and notifies the vessel.
         */
        void setVessel(Vessel* vessel);
};


#endif
