#include <iostream>
#include <cstdint>

#include "../../core/id_manager.hpp"
#include "../../core/common.hpp"

int main(){
    std::uint32_t id1, id2, id3, id4, id5;
    short res;

    res = create_id(id1, VESSEL_SET);
    std::cout << "create_id returned " << res << std::endl;

    res = create_id(id2, VESSEL_SET);
    std::cout << "create_id returned " << res << std::endl;

    res = create_id(id3, VESSEL_SET);
    std::cout << "create_id returned " << res << std::endl;

    res = add_id(1, VESSEL_SET);
    std::cout << "add_id returned " << res << std::endl;

    res = add_id(1, VESSEL_SET);
    std::cout << "add_id returned " << res << std::endl;

    res = add_id(1, PART_SET);
    std::cout << "add_id returned " << res << std::endl;

    res = add_id(2, PART_SET);
    std::cout << "add_id returned " << res << std::endl;

    res = add_id(3, PART_SET);
    std::cout << "add_id returned " << res << std::endl;

    res = remove_id(2, PART_SET);
    std::cout << "remove_id returned " << res << std::endl;

    res = create_id(id4, PART_SET);
    std::cout << "create_id returned " << res << std::endl;

    res = remove_id(4, PART_SET);
    std::cout << "remove_id returned " << res << std::endl;

    res = remove_id(2, 3);
    std::cout << "remove_id returned " << res << std::endl;


    std::cout << "id1: " << id1 << " id2: " << id2 << " id3: " << id3 << " id4: " << id4 << std::endl;
    print_set(VESSEL_SET);
    print_set(PART_SET);


    return 0;
}
