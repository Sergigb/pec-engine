#ifndef VESSEL_HPP
#define VESSEL_HPP

#include <string>
#include <cstdint>
#include <map>
#include <vector>

#include "BasePart.hpp"
#include "id_manager.hpp"
#include "log.hpp"
#include "BtWrapper.hpp" // collision flags


class Vessel{
    private:
        std::string m_vessel_name, m_vessel_desc;
        std::shared_ptr<BasePart> m_vessel_root;
        std::vector<BasePart*> m_node_list;
        std::map<std::uint32_t, BasePart*> m_node_map_by_id;
        std::uint32_t m_vessel_id;

        void updateNodes(); // updates node list and map
    public:
        Vessel();
        Vessel(std::shared_ptr<BasePart>& vessel_root);
        ~Vessel();

        void setVesselName(const std::string& name);
        void setVesselDescription(const std::string& description);
        //void setRoot(BasePart* part);

        /* These functions should only be used in the editor */
        bool addChildById(std::shared_ptr<BasePart>& child, std::uint32_t parent_id);
        bool addChild(BasePart* child, BasePart* parent);
        std::shared_ptr<BasePart> removeChild(BasePart* child);
        std::shared_ptr<BasePart> removeChildById(std::uint32_t child_id);

        void onTreeUpdate();

        BasePart* getRoot() const;
        BasePart* getPartById(std::uint32_t id) const;
        const std::string getVesselName() const;
        const std::string getVesselDescription() const;
        std::vector<BasePart*>* getParts();
        std::uint32_t getId() const;
        void printVessel() const;
};


#endif