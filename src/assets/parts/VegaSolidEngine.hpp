#ifndef VegaSolidEngine_HPP
#define VegaSolidEngine_HPP

#include <memory>

#include "../BasePart.hpp"
#include "../subcomponents/EngineComponent.hpp"


class VegaSolidEngine : public BasePart{
    private:
        std::uint32_t m_htpb_id;
        double m_separation_force;
        bool m_separate;

        EngineComponent* m_main_engine;

        Model* m_fairing_model;

        void init();
    public:
        VegaSolidEngine(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        VegaSolidEngine(const VegaSolidEngine& engine);
        VegaSolidEngine();
        ~VegaSolidEngine();

        void renderOther();
        void update();
        VegaSolidEngine* clone() const;
        void action(int action);

        int render(const math::mat4& body_transform);
        int render();

        int loadCustom(const tinyxml2::XMLElement* elem);
};



#endif
