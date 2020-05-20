#include <cassert>
#include <iostream>

#include "BasePart.hpp"


BasePart::BasePart(){
	m_parent = nullptr;
}

BasePart::BasePart(const std::string& name, const std::string& full_name, double mass, unsigned int type, bool root) : 
	m_part_mass(mass), m_key_name(name), m_fancy_name (full_name), m_part_class_type(type), m_is_root(root){
		m_id = reinterpret_cast<std::uintptr_t>(this);
	}


BasePart::BasePart(BasePart& old){
	m_parent = nullptr;
	m_part_mass = old.m_part_mass;
	m_key_name = old.m_key_name;
	m_fancy_name = old.m_fancy_name;
	m_part_class_type = old.m_part_class_type;
	m_id = reinterpret_cast<std::uintptr_t>(this);
	m_is_root = old.m_is_root;
}

BasePart::~BasePart(){}

void BasePart::appendChild(BasePart *child){
	child->setParent(this);
	m_children.push_back(child);
}

void BasePart::setParent(BasePart *parent_){
	m_parent = parent_;
}

bool BasePart::hasChildren() const{
	return(m_children.size() > 0);
}

bool BasePart::hasParent() const{
	return(m_parent != NULL);
}

bool BasePart::isRoot() const{
	return m_is_root;
}

BasePart* BasePart::getParent() const{
	return m_parent;
}

BasePart* BasePart::getChild(const unsigned int pos) const{
	assert(pos < m_children.size());

    if(pos > m_children.size())
        return NULL;
    else
        return m_children[pos];
}

unsigned int BasePart::childrenNumber() const{
    return m_children.size();
}

double BasePart::getPartMass() const{
	return m_part_mass;
}

const std::string& BasePart::getPartFancyName() const{
	return m_fancy_name;
}

const std::string& BasePart::getPartKeyName() const{
	return m_key_name;
}

void BasePart::updateState(){
	
}

uintptr_t BasePart::getId() const{
	return m_id;
}

void BasePart::setAsRoot(){
	m_is_root = true;
}

