#ifndef VESSEL_HPP
#define VESSEL_HPP

#include <string>
#include <cstdint>
#include <map>
#include <vector>

#include "BasePart.hpp"
#include "id_manager.hpp"


class Vessel{
    private:
        std::string m_vessel_name, m_vessel_desc;
        BasePart* m_vessel_root;
        std::vector<BasePart*> m_node_list;
        std::map<std::uint32_t, BasePart*> m_node_map_by_id;

        void updateNodes(); // updates node list and map
    public:
        Vessel();
        Vessel(BasePart* part);
        ~Vessel();

        void setVesselName(const std::string& name);
        void setVesselDescription(const std::string& description);
        void setRoot(BasePart* part);

        /*
         * When we want to append a child in, for example, the editor, we get the part pointer (that's the user 
         * pointer) in the callback returned by rayTest. We attach the child directly using the parts interface,
         * and we don't call Vessel at any moment. Not noticing Vessel can be harmful if for example we don't update
         * the pointer to the Vessel it belongs to, or the parts list/map. For that reason, every time we append
         * a child to a vessel we MUST call the onChildAppend. Not sure if this situation is ideal, but it'll 
         * suffice for now.
         */
        void onChildAppend(std::uint32_t id);

        BasePart* getRoot() const;
        BasePart* getPartById(std::uint32_t id) const;
        const std::string getVesselName();
        const std::string getVesselDescription();
};


#endif