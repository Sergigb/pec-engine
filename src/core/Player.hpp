#ifndef PLAYER_HPP
#define PLAYER_HPP


#define PLAYER_BEHAVIOUR_NONE 0
#define PLAYER_BEHAVIOUR_EDITOR 1
#define PLAYER_BEHAVIOUR_SIMULATION 2


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
        short m_behaviour;

        void setCamMode();
        void switchVessel();
    public:
        Player(Camera* camera, AssetManager* asset_manager, const Input* input);
        ~Player();

        void update();

        // this method should be called by the player controlled vessel
        void onVesselDestroy();
        void setBehaviour(short behaviour);

        Vessel* getVessel() const;
};


#endif
