#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <map>

#include "Ship.hpp"
#include "BasePart.hpp"
#include "parts/parts.hpp"
#include "common.hpp"


partsMap* init_parts(){
	partsMap *parts = new partsMap();
 	(*parts)["cm_module"] = new CommandModule("cm_module", "Command module", 100.0, 0, false); //command module
	(*parts)["sv_module"] = new ServiceModule("sv_module", "Service module", 100.0, 0, false); // service module
	(*parts)["lea"] = new LaunchEscapeAssembly("lea", "Launch Escape Assembly", 10.0, 0, false); // space launch assembly
	(*parts)["sla"] = new BasePart("sla", "Spacecraft Lunar Module Adapter", 150.0, 0, false); //generic part??
	(*parts)["sivb_booster"] = new ResourceContainer("sivb_booster", "S-IVB Booster", 400.0, 0, false); //generic resource container
	(*parts)["j2"] = new Engine("j2_engine", "J-2 engine", 50.0, 0, false); // generic engine
	(*parts)["sii_booster"] = new ResourceContainer("sii_booster", "S-II Booster", 1000.0, 0, false); //generic resource container

	return parts;
}


void print_parts(partsMap *parts){
	std::cout << std::endl << "List of available parts: " << std::endl;

	for(partsMap::iterator it = parts->begin(); it != parts->end(); it++){
		std::cout << "\t- " << (*parts)[it->first]->getPartFancyName() << " (key: " << it->first << ")" << std::endl;
	}
	std::cout << std::endl;
}


void ship_editor(Ship &ship, partsMap *parts){
	std::string ship_name, part_key, option_str;
	bool exit = false;
	int option = OPTION_PRINT_OPTIONS, part_index;
	BasePart *part;

	std::cout << "Name of the ship: ";
	std::getline(std::cin, ship_name);
	ship.setShipName(ship_name);

	print_parts(parts);
	std::cout << "Pick a part to set as root: ";
	std::getline(std::cin, part_key);

	try{
		ship.setRoot(new BasePart(*parts->at(part_key)));
	}
	catch(const std::out_of_range &err){
		std::cerr << "Exception when using " << err.what() << " (key out of range)" << std::endl;
	}

	while(!exit){
		std::cout << "Enter an option (enter 7 to show all options): ";
		std::getline(std::cin, option_str);
		try{
			option = std::stoi(option_str);
		}
		catch(const std::exception &err){
			std::cout << "Exception when using " << err.what() << std::endl;
			option = OPTION_INVALID;
		}
		switch(option){
			case OPTION_PRINT_OPTIONS:
				std::cout << "Editor options: " << std::endl;
				std::cout << "\t0 - Exits editor" << std::endl;
				std::cout << "\t1 - Appends node" << std::endl;
				std::cout << "\t2 - Deletes node" << std::endl;
				std::cout << "\t3 - Changes ship's name" << std::endl;
				std::cout << "\t4 - Changes ship's root" << std::endl;
				std::cout << "\t5 - Show all available parts" << std::endl;
				std::cout << "\t6 - Prints ship's part tree" << std::endl;
				std::cout << "\t7 - Prints this menu" << std::endl;
				break;
			case OPTION_SHOW_PARTS:
				print_parts(parts);
				break;
			case OPTION_APPEND_NODE:
				std::cout << "Pick a part to append: ";
				std::getline(std::cin, part_key);
				try{
					part = new BasePart(*parts->at(part_key));
				}
				catch(const std::exception &err){
					std::cout << "Exception when using " << err.what() << std::endl;
				}
				std::cout << "List of nodes: " << std::endl;
				ship.printNodeList();
				
				std::cout << "Pick the index of the part to attach the new part to: ";
				std::getline(std::cin, option_str);
				try{
					part_index = std::stoi(option_str);
				}
				catch(const std::exception &err){
					std::cout << "Exception when using " << err.what() << std::endl;
					break;
				}
				if(ship.appendChildByIndex(part_index, part) == EXIT_FAILURE)
					std::cout << "Part could not be attached" << std::endl;
				break;
			case OPTION_DELETE_NODE:
				break;
			case OPTION_CHANGE_NAME:
				std::cout << "Name of the ship: ";
				std::getline(std::cin, ship_name);
				ship.setShipName(ship_name);
				break;
			case OPTION_PRINT_TREE:
				ship.printTree();
				break;
			case OPTION_EXIT:
				exit = true;
				break;
			default:
				std::cout << "Invalid option" << std::endl;
		}
	}

	ship.printTree();
}

int main(){
	Ship ship;
	partsMap *parts = init_parts();

	ship_editor(ship, parts);

	while(1)
		ship.printPathBetweenNodes();

	delete parts;

	return EXIT_SUCCESS;
}