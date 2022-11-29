#ifndef VESSEL_HPP
#define VESSEL_HPP

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>


class Player;
class BasePart;
class Input;
class btVector3;


/*
 * Structs that holds an action of a stage of the vessel. It has a part as a target and
 * the action.
 *
 * @part: pointer to the part.
 * @action: action that the part should perform.
 */
struct stage_action{
    BasePart* part;
    long action;
    stage_action(BasePart* ptr, long act){
        action = act;
        part = ptr;
    }
};


/*
 * Vessel class, holds the vessel tree and other information. The player should is able to control
 * vessels.
 */
class Vessel{
    private:
        std::string m_vessel_name, m_vessel_desc;
        std::shared_ptr<BasePart> m_vessel_root;
        std::vector<BasePart*> m_node_list;
        std::unordered_map<std::uint32_t, BasePart*> m_node_map_by_id;
        std::uint32_t m_vessel_id;
        float m_yaw, m_pitch;
        double m_total_mass;
        btVector3 m_com;
        std::vector<std::vector<stage_action>> m_stages;

        Player* m_player; // player controlling the vessel
        const Input* m_input;

        /*
         * Updates m_node_list and m_node_map_by_id by traveling through the vessel tree.
         */
        void updateNodes();

        /*
         * Updates the mass of the vessel (m_total_mass). This is not used by bullet buy it may
         * be used by other physics calculations.
         */
        void updateMass();

        /*
         * Crudely updates the staging of the vessel.
         */
        void updateStaging();

        /*
         * Recursive method used by updateStaging.
         */

        void updateStagingRec(BasePart* part);

        /*
         * Triggers the next stage of the vessel.
         */
        void activateNextStage();
    public:
        Vessel();

        /*
         * Constructor.
         *
         * @vessel_root: root of the vessel, the vessel will take ownership of this pointer.
         * @input: pointer to the input object.
         */
        Vessel(std::shared_ptr<BasePart>&& vessel_root, const Input* input);

        ~Vessel();

        /*
         * Sets the vessel name.
         */
        void setVesselName(const std::string& name);

        /*
         * Sets the vessel description.
         */
        void setVesselDescription(const std::string& description);

        //void setRoot(BasePart* pat);

        /*
         * Sets the player pointer, this means that the particular player is controlling this
         * vessel.
         *
         * @player: pointer to the player object.
         */
        void setPlayer(Player* player);

        /*
         * Sets the velocity vector of the vessel, use with caution.
         *
         * @velocity: new velocity value.
         */
        void setVesselVelocity(const btVector3& velocity);

        /*
         * Attaches a child to a parent node using its ID. Returns false if the parent ID is not
         * found in the tree.
         *
         * @child: rvalue reference to the shared pointer of the child, the parent will take 
         * ownership of this pointer.
         * @parent_id: id of the parent we want to attach the child to.
         */
        bool addChildById(std::shared_ptr<BasePart>&& child, std::uint32_t parent_id);

        /*
         * Nasty method that attaches a child to the referred parent pointer. I don't think this is
         * used anywhere and to be hones I don't like it. The method uses the getSharedPtr to
         * obtain the shared pointer. Returns false if the parent doesn't belong to the tree.
         *
         * @child: raw pointer to the child.
         * @parent: raw pointer to the parent.
         */
        bool addChild(BasePart* child, BasePart* parent);

        /*
         * Removes a child and returns its shared pointer. I don't like that we pass a raw pointer.
         * removeChildById seems cleaner and more safe. Returns a shared pointer to null if the
         * child is not found.
         *
         * @child: raw pointer to the child.
         */
        std::shared_ptr<BasePart> removeChild(BasePart* child);

        /*
         * Removes a child and returns its shared pointer using its ID. Returns a shared pointer
         * to null if the child is not found.
         *
         * @child_id: ID of the child.
         */
        std::shared_ptr<BasePart> removeChildById(std::uint32_t child_id);

        /*
         * Method that should be called each time the tree updates. This should be mainly called
         * by the vessel's nodes.
         */
        void onTreeUpdate();

        /*
         * Returns a pointer to the root of the tree.
         */
        BasePart* getRoot();

        /*
         * Returns a pointer to a node of the tree using its ID.
         *
         * @id: ID of the part.
         */
        BasePart* getPartById(std::uint32_t id);

        /*
         * Const version of getRoot.
         */
        const BasePart* getRoot() const;

        /*
         * Const version of getPartById.
         */
        const BasePart* getPartById(std::uint32_t id) const;

        /*
         * Returns the name of the vessel.
         */
        const std::string& getVesselName() const;

        /*
         * Returns the description of the vessel.
         */
        const std::string& getVesselDescription() const;

        /*
         * Returns a reference to the list of parts of the vessel.
         */
        std::vector<BasePart*>& getParts();

        /*
         * Const version of getParts.
         */
        const std::vector<BasePart*>& getParts() const;

        /*
         * Returns the ID of the vessel.
         */
        std::uint32_t getId() const;

        /*
         * Prints the vessel tree via stdout.
         */
        void printVessel() const;

        /*
         * Returns the input pointer, I think it's used by the parts.
         */
        const Input* getInput() const;

        /*
         * Returns the yaw input of the vessel.
         */
        float getYaw() const;

        /*
         * Returns the pitch input of the vessel.
         */
        float getPitch() const;

        /*
         * Returns the total mass of the vessel.
         */
        double getTotalMass() const;

        /*
         * Returns the center of mass (global) of the vessel.
         */
        const btVector3& getCoM() const;

        /*
         * Prints the staging of the vessel.
         */
        void printStaging() const;

        /*
         * Returns some distance that is supposed to be I think the height between the vessel's
         * root origin and it's lowest point.
         */
        double getLowerBound() const;

        /*
         * Updates stuff like the mass and the subtree.
         */
        void update();

        /*
         * Returns staging, input, subtree, etc.
         */
        void updateCoM();
};


#endif