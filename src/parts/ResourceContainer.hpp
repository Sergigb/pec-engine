#include "../BasePart.hpp"


class ResourceContainer : public BasePart{

	private:


	public:
		ResourceContainer();
		ResourceContainer(const std::string& name,  const std::string& full_name, double mass, unsigned int type, bool root);
		~ResourceContainer();

		void updateState();

};

