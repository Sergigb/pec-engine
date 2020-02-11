#include "../BasePart.hpp"


class Engine : public BasePart{

	private:


	public:
		Engine();
		Engine(const std::string& name, const std::string& full_name, double mass, unsigned int type, bool root);
		~Engine();

		void updateState();

};

