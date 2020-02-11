#include "../BasePart.hpp"


class CommandModule : public BasePart{

	private:


	public:
		CommandModule();
		CommandModule(const std::string& name, const std::string& full_name, double mass, unsigned int type, bool root);
		~CommandModule();

		void updateState();

};


