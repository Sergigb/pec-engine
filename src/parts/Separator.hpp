#ifndef SEPARATOR_HPP
#define SEPARATOR_HPP

#include "../BasePart.hpp"


#define BEHAVIOUR_SEPARATES_CHILDS 1
#define BEHAVIOUR_SEPARATES_SELF 2
#define BEHAVIOUR_SEPARATES_ALL 3


class Separator : public BasePart{
    private:
        char m_behaviour;
        bool m_separate;
    public:
        Separator(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        Separator(const Separator& engine);
        Separator();
        ~Separator();

        void renderOther();
        void update();
        Separator* clone() const;
};

#endif
