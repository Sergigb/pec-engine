#ifndef PLAYER_HPP
#define PLAYER_HPP


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
    public:
        Player(Camera* camera, AssetManager* asset_manager, Input* input);
        ~Player();

        void update();
};


#endif
