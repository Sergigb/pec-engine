#include "../BasePart.hpp"


class ServiceModule : public BasePart{

	private:


	public:
		ServiceModule();
		ServiceModule(const std::string& name,  const std::string& full_name, double mass, unsigned int type, bool root);
		~ServiceModule();

		void updateState();

};


