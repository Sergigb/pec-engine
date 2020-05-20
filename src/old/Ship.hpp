#ifndef SHIP_HPP
#define SHIP_HPP

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "BasePart.hpp"


class Ship{
	private:
		std::string m_ship_name;
		BasePart* m_ship_root;
		std::vector<BasePart*> m_node_list;
		std::map<uintptr_t, BasePart*> m_node_map_by_id;

		std::unique_ptr<std::vector<BasePart*>> findPathToRoot(BasePart* node) const;
		std::unique_ptr<std::vector<BasePart*>> findPathBetweenNodes(BasePart* node1, BasePart* node2) const;
		int updateNodes();

	public:
		Ship();
		Ship(const std::string& name);
		~Ship(); 

		void setShipName(const std::string& name);
		void setRoot(BasePart* part);
		int appendChildById(const uintptr_t id, BasePart* child);
		int appendChildByIndex(const uint pos, BasePart* child);

		const std::string* getShipName() const;

		void printTree() const;
		void printNodeList() const;
		void printPathBetweenNodes() const;
		
};
#endif
