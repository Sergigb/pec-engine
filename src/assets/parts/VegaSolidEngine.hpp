#ifndef VegaSolidEngine_HPP
#define VegaSolidEngine_HPP

#include <memory>

#include "../BasePart.hpp"


#define ENGINE_OFF 0
#define ENGINE_ON 1
#define ENGINE_DEPLETED 2


class VegaSolidEngine : public BasePart{
    private:
        short m_engine_status;
        std::uint32_t m_htpb_id;
        double m_average_thrust, m_mass_flow_rate, m_max_deflection_angle, m_separation_force;
        bool m_separate;

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

        int render(math::mat4 body_transform);
        int render();

        void setFairingModel(Model* fairing_model);
        void setEngineStats(double average_thrust, double mass_flow_rate, double max_deflection_angle);
        void setSeparationForce(double force);
};



#endif
