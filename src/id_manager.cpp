#include <unordered_set>
#include <random>
#include <limits>
#include <iostream>

#include "id_manager.hpp"


struct idmanager{
    std::unordered_set<std::uint32_t> vessels_id_set;
    std::unordered_set<std::uint32_t> parts_id_set;
};

static idmanager manager;
static std::default_random_engine generator(0);
static std::uniform_int_distribution<std::uint32_t> distribution(0, std::numeric_limits<uint32_t>::max());


char pick_set(short set, std::unordered_set<std::uint32_t>*& ptr){
    if(set == VESSEL_SET){
        ptr = &manager.vessels_id_set;
        return 0;
    }
    else if(set == PART_SET){
        ptr = &manager.parts_id_set;
        return 0;
    }
    else{
        return 1;
    }    
}


short add_id(std::uint32_t id, short set){
    std::unordered_set<std::uint32_t>* uset_ptr;

    if(pick_set(set, uset_ptr)){
        return REQUEST_INVALID_SET;
    }

    if(id == 0){
        return REQUEST_ID_INVALID;
    }

    std::unordered_set<std::uint32_t>::iterator res = uset_ptr->find(id);
    if(res != uset_ptr->end()){
        return REQUEST_ID_COLLISION;
    }

    uset_ptr->insert(id);

    return REQUEST_SUCCESS;
}


short remove_id(std::uint32_t id, short set){
    std::unordered_set<std::uint32_t>* uset_ptr;

    if(pick_set(set, uset_ptr)){
        return REQUEST_INVALID_SET;
    }

    std::unordered_set<std::uint32_t>::iterator res = uset_ptr->find(id);
    if(res == uset_ptr->end()){
        return REQUEST_ID_MISSING;
    }

    uset_ptr->erase(id);

    return REQUEST_SUCCESS;
}


short create_id(std::uint32_t& id, short set){
    std::unordered_set<std::uint32_t>* uset_ptr;

    if(pick_set(set, uset_ptr)){
        return REQUEST_INVALID_SET;
    }

    do{
        id = distribution(generator);
    }while(uset_ptr->find(id) != uset_ptr->end());

    uset_ptr->insert(id);
    
    return REQUEST_SUCCESS;
}


short print_set(short set){
    std::unordered_set<std::uint32_t>* uset_ptr;
    std::unordered_set<std::uint32_t>::iterator it;

    if(pick_set(set, uset_ptr)){
        return REQUEST_INVALID_SET;
    }

    it = uset_ptr->begin();

    while(it != uset_ptr->end()){
        std::cout << *it << " ";
        it++;
    }
    std::cout << std::endl;

    return REQUEST_SUCCESS;
}

