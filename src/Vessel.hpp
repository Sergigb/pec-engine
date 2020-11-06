#ifndef VESSEL_HPP
#define VESSEL_HPP

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <memory>


class Player;
class BasePart;
class Input;


class Vessel{
    private:
        std::string m_vessel_name, m_vessel_desc;
        std::shared_ptr<BasePart> m_vessel_root;
        std::vector<BasePart*> m_node_list;
        std::map<std::uint32_t, BasePart*> m_node_map_by_id;
        std::uint32_t m_vessel_id;
        float m_yaw, m_pitch;

        Player* m_player; // player controlling the vessel
        const Input* m_input;

        void updateNodes(); // updates node list and map
    public:
        Vessel();
        Vessel(std::shared_ptr<BasePart>& vessel_root, const Input* input);
        ~Vessel();

        void setVesselName(const std::string& name);
        void setVesselDescription(const std::string& description);
        //void setRoot(BasePart* pat);
        void setPlayer(Player* player);

        /* These functions should only be used in the editor */
        bool addChildById(std::shared_ptr<BasePart>& child, std::uint32_t parent_id);
        bool addChild(BasePart* child, BasePart* parent);
        std::shared_ptr<BasePart> removeChild(BasePart* child);
        std::shared_ptr<BasePart> removeChildById(std::uint32_t child_id);

        void onTreeUpdate();

        BasePart* getRoot();
        BasePart* getPartById(std::uint32_t id);
        const BasePart* getRoot() const;
        const BasePart* getPartById(std::uint32_t id) const;
        const std::string getVesselName() const;
        const std::string getVesselDescription() const;
        std::vector<BasePart*>* getParts();
        const std::vector<BasePart*>* getParts() const;
        std::uint32_t getId() const;
        void printVessel() const;
        const Input* getInput() const;
        float getYaw() const;
        float getPitch() const;

        void update();
};


#endif