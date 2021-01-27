#ifndef GENERIC_ENGINE_HPP
#define GENERIC_ENGINE_HPP


#include "../BasePart.hpp"


class GenericEngine : public BasePart{
    private:
        bool m_engine_status;
        float m_thrust;
        std::uint32_t m_liquid_hydrogen_id, m_liquid_oxygen_id;
    public:
        GenericEngine(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        GenericEngine(const GenericEngine& engine);
        GenericEngine();
        ~GenericEngine();

        void renderOther();
        void update();
        GenericEngine* clone() const;
        void action(int action);
};



#endif
