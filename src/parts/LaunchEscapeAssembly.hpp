#include "../BasePart.hpp"


class LaunchEscapeAssembly : public BasePart{

	private:


	public:
		LaunchEscapeAssembly();
		LaunchEscapeAssembly(const std::string& name, const std::string& full_name, double mass, unsigned int type, bool root);
		~LaunchEscapeAssembly();

		void updateState();

};

