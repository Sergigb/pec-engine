#ifndef GENERIC_ENGINE_HPP
#define GENERIC_ENGINE_HPP


#include "../BasePart.hpp"


class GenericEngine : public BasePart{
    private:
        bool m_engine_status;
        float m_thrust;
    public:
        GenericEngine(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        GenericEngine(const GenericEngine& engine);
        GenericEngine();
        ~GenericEngine();

        void renderOther();
        void update();
        GenericEngine* clone() const;
};



#endif
