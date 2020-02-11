#ifndef PART_HPP
#define PART_HPP

#include <string>
#include <vector>
#include <ctime>
#include <cstdint>

// Interface class for parts

class BasePart {
	protected:
		std::vector<BasePart *> m_children;
		BasePart* m_parent;

		double m_part_mass;
		std::string m_key_name;
		std::string m_fancy_name;
		unsigned int m_part_class_type;
		uintptr_t m_id;
		bool m_is_root;

	public:
		BasePart();
		BasePart(const std::string &name, const std::string &full_name, double mass, unsigned int type, bool root);
		BasePart(BasePart& old);
		virtual ~BasePart();

		void appendChild(BasePart *child);
		void setParent(BasePart *child);
		void setAsRoot();

		bool hasChildren() const;
		bool hasParent() const;
		bool isRoot() const;

		BasePart* getParent() const;
		BasePart* getChild(const unsigned int pos) const;
		//std::vector<BasePart *>* getChilds

		unsigned int childrenNumber() const;
		double getPartMass() const;
		uintptr_t getId() const;
		const std::string& getPartFancyName() const;
		const std::string& getPartKeyName() const;

		//part interface
		virtual void updateState();
};

#endif
