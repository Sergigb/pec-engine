#ifndef PLAYER_HPP
#define PLAYER_HPP


#define PLAYER_BEHAVIOUR_NONE 0x01
#define PLAYER_BEHAVIOUR_EDITOR 0x02
#define PLAYER_BEHAVIOUR_SIMULATION 0x04
#define PLAYER_BEHAVIOUR_PLANETARIUM 0x08

/* Camera orbit mode */
#define ORBITAL_CAM_MODE_SURFACE 0
#define ORBITAL_CAM_MODE_ORBIT 1

#define PLANETARIUM_DEF_SCALE_FACTOR 1e10

class Vessel;
class Camera;
class AssetManager;
class Input;
class BaseApp;


/*
 * Player class, has code that can be used in different applications (camera update, for example),
 * also holds information such as the vessel that the player is controlling or the planet selected
 * in the planetarium view.
 *
 * Note about this class: I think it would be easier to let the application manage the player
 * update method, which is essentially input + camera. We should make some methods of this class,
 * such as setPlayerTarget, public and let the application decide when it wants to call then.
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
        bool m_planetarium_freecam;
        double m_planetarium_scale_factor;

        void togglePlanetariumFreecam();
        void unsetVessel();
        void setPlayerTarget();
        void switchVessel();
        //void switchPlanet(); see setSelectedPlanet
        void setCamAxisRotation();
    public:
        Player(Camera* camera, AssetManager* asset_manager, const Input* input);
        ~Player();

        /*
         * Update method for the editor.
         */
        void updateEditor();

        /*
         * Update method for the simulation (main game).
         */
        void updateSimulation();

        /*
         * Update method for the planetariun.
         */
        void updatePlanetarium();

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
         * Sets the scale of the planetarium, rendering of the planetarium uses a smaller scale
         * because big numbers break everything (text/orbits), we need this scale factor for the
         * camera. The default value is 1e10, you probably don't need to change this value.
         *
         * @factor: scale factor.
         */
        void setPlanetariumScaleFactor(double factor);

        /*
         * Returns a raw pointer to the user controlled vessel.
         */
        Vessel* getVessel() const;

        /*
         * Returns true if the player is using the free camera while on the planetarium view.
         */
        bool getPlanetariumFreecam() const;
};


#endif
