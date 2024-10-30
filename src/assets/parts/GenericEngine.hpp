#ifndef GENERIC_ENGINE_HPP
#define GENERIC_ENGINE_HPP


#include "../BasePart.hpp"
#include "../subcomponents/EngineComponent.hpp"


/*
 * Generic engine, mostly used for testing engines.
 */
class GenericEngine : public BasePart{
    private:
        EngineComponent m_main_engine;
    public:
        GenericEngine(Model* model, Physics* physics, btCollisionShape* col_shape, 
                      btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        GenericEngine(const GenericEngine& engine);
        GenericEngine();
        ~GenericEngine();

        void renderOther();

        void update();

        GenericEngine* clone() const;

        void action(int action);
};



#endif
