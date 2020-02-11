#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

#include "Ship.hpp"


Ship::Ship(){
	m_ship_root = nullptr;
	m_ship_name = "";
}
Ship::Ship(const std::string& name): m_ship_name(name){
	m_ship_root = nullptr;
}
Ship::~Ship(){
	for(unsigned int i = 0; i < m_node_list.size(); i++){
		delete m_node_list[i];
	}
}

int Ship::updateNodes(){
	if(m_ship_root == nullptr){
		std::cerr << "Ship::updateNodes - Ship has no root, nothing to update" << std::endl;
		return EXIT_FAILURE;
	}
	std::vector<BasePart *> to_visit;
	BasePart* current = m_ship_root;
	
	m_node_list.clear();
	m_node_map_by_id.clear();

	to_visit.insert(to_visit.end(), current);
	m_node_list.insert(m_node_list.end(), current);
	m_node_map_by_id[current->getId()] = current;

	while(!to_visit.empty()){
		for(unsigned int i = 0; i < current->childrenNumber(); i++){
			m_node_list.insert(m_node_list.end(), current->getChild(i));
			m_node_map_by_id[current->getChild(i)->getId()] = current->getChild(i);
			to_visit.insert(to_visit.end(), current->getChild(i));
		}
		current = to_visit.back();
		to_visit.pop_back();
	}
	return EXIT_SUCCESS;
}

const std::string* Ship::getShipName() const
{
	return &m_ship_name;
}

std::string vertical_and_right = "\u251C";
std::string horizontal = "\u2500\u2500\u2500";
std::string vertical = "\u2502";
std::string up_and_right = "\u2514";
std::string separator = "    ";

void print_tree_member(BasePart *node, std::string tail, bool last_child){
	const std::string &part_name = node->getPartFancyName();
    unsigned int num_childs = node->childrenNumber();

	std::string next_tail;
	if(!last_child){
		std::cout << tail << up_and_right  << horizontal << " " << part_name << " - " << node->getId() << std::endl;
		next_tail = tail + " " + separator;
	}
	else{
		std::cout << tail << vertical_and_right  << horizontal << " " << part_name << " - " << node->getId() << std::endl;
		next_tail = tail + vertical + separator;
	}

	for(unsigned int i = 0; i < num_childs; i++)
		print_tree_member(node->getChild(i), next_tail, !(i == num_childs - 1));
}

void Ship::printTree() const{
	if(m_ship_root == nullptr){
		std::cerr << "Ship::printTree - ship has no part tree" << std::endl;
		return;
	}
	std::string tail = "";
	std::cout << m_ship_name << "'s part tree:" << std::endl;
	print_tree_member(m_ship_root, tail, false);
}

std::unique_ptr<std::vector<BasePart *>> Ship::findPathToRoot(BasePart *node) const{
	std::unique_ptr<std::vector<BasePart *>> path_to_root(new std::vector<BasePart *>());
	path_to_root->push_back(node);

	BasePart *current = node;
	while(!current->isRoot()){
		current = current->getParent();
		path_to_root->push_back(current);
	}

	return path_to_root;
}

std::unique_ptr<std::vector<BasePart *>> Ship::findPathBetweenNodes(BasePart* node1, BasePart* node2) const{
	std::unique_ptr<std::vector<BasePart *>> path_node1_root = findPathToRoot(node1);
	std::unique_ptr<std::vector<BasePart *>> path_node2_root = findPathToRoot(node2);
	unsigned int min_size, i;

	if(path_node1_root->size() < path_node2_root->size())
		min_size = path_node1_root->size();
	else
		min_size = path_node2_root->size();

	i  = min_size;
	if(i != 0)
		while(path_node1_root->at(path_node1_root->size() - i) != path_node2_root->at(path_node2_root->size() - i))
			i--;

	while(i > 1){
		path_node1_root->erase(path_node1_root->end() - 1);
		path_node2_root->erase(path_node2_root->end() - 1);
		i--;
	}
	// the ending node is always repeated in both paths, so we delete the last one of the first list
	path_node1_root->pop_back();

	std::reverse(path_node2_root->begin(), path_node2_root->end());
	path_node1_root->insert(path_node1_root->end(), path_node2_root->begin(), path_node2_root->end());

	return path_node1_root;
}

void Ship::printNodeList() const{
	for(unsigned int i = 0; i < m_node_list.size(); i++){
		const std::string &part_name = m_node_list[i]->getPartFancyName(); 
		uintptr_t id = m_node_list[i]->getId();
		std::cout << i << " - " << part_name << " (" << id << ")" << std::endl;
	}
}


void Ship::printPathBetweenNodes() const{
	unsigned int num_parts = m_node_list.size(), node1_index, node2_index, num_items;
	std::string index;
	std::unique_ptr<std::vector<BasePart *>> path;

	std::cout << "List of parts: " << std::endl;
	printNodeList();
	do{
		std::cout << "Index of the first part: ";
		std::cin >> index;
		node1_index = std::stoi(index);
	}while(node1_index > num_parts);
	do{
		std::cout << "Index of the second part: ";
		std::cin >> index;
		node2_index = std::stoi(index);
	}while(node2_index > num_parts);

	std::cout << "Path between nodes \"" << m_node_list[node1_index]->getPartFancyName() << "\" and \"" 
			  << m_node_list[node2_index]->getPartFancyName() << "\": " << std::endl;
	
	path = findPathBetweenNodes(m_node_list[node1_index], m_node_list[node2_index]);

	num_items = path->size();
	for(unsigned int i = 0; i < num_items; i++){
		std::cout << path->at(i)->getPartFancyName();
		if(i != num_items - 1)
			std::cout << " -> ";
	}
	std::cout << std::endl << std::endl;
}


int Ship::appendChildById(const uintptr_t id, BasePart *child){
	try{
		m_node_map_by_id.at(id)->appendChild(child);
	}
	catch(const std::out_of_range &err){
		std::cerr << "Ship::appendChildById - " << err.what() << std::endl;
		return EXIT_FAILURE;
	}
	updateNodes();

	return EXIT_SUCCESS;
}


int Ship::appendChildByIndex(const uint pos, BasePart *child){
	try{
		m_node_list.at(pos)->appendChild(child);
	}
	catch(const std::out_of_range &err){
		std::cerr << "Ship::appendChildByIndex - " << err.what() << std::endl;
		return EXIT_FAILURE;
	}
	updateNodes();

	return EXIT_SUCCESS;
}


void Ship::setShipName(const std::string &name){
	m_ship_name = name;
}


void Ship::setRoot(BasePart *part){
	m_ship_root = part;
	m_ship_root->setAsRoot();
	updateNodes();
}
