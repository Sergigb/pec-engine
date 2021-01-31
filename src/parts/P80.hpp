#ifndef P80_HPP
#define P80_HPP


#include "../BasePart.hpp"


// the actual thrust pressure profile is more complex, but the average is a good approximation for now
#define AVERAGE_THRUST 2271000
#define MASS_FLOW_RATE 828.35 // kg/s

#define MAX_DEFLECTION_ANGLE 6.5 * ONE_DEG_IN_RAD

#define ENGINE_OFF 0
#define ENGINE_ON 1
#define ENGINE_DEPLETED 2


class P80 : public BasePart{
    private:
        short m_engine_status;
        std::uint32_t m_htpb_id;
    public:
        P80(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        P80(const P80& engine);
        P80();
        ~P80();

        void renderOther();
        void update();
        P80* clone() const;
        void action(int action);
};



#endif
