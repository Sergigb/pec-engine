#ifndef PLAYER_HPP
#define PLAYER_HPP


#define PLAYER_BEHAVIOUR_NONE 0
#define PLAYER_BEHAVIOUR_EDITOR 1
#define PLAYER_BEHAVIOUR_SIMULATION 2


class Vessel;
class Camera;
class AssetManager;
class Input;


class Player{
    private:
        Vessel* m_vessel; // user controlled vessel
        Camera* m_camera;
        AssetManager* m_asset_manager;
        const Input* m_input;
        short m_behaviour;
    public:
        Player(Camera* camera, AssetManager* asset_manager, Input* input);
        ~Player();

        void update();

        // this method should be called by the player controlled vessel
        void onVesselDestroy();
        void setBehaviour();
};


#endif
