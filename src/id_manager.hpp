#ifndef ID_MANAGER_HPP
#define ID_MANAGER_HPP

#include <cstdint>

#include "common.hpp"

#define VESSEL_SET 1
#define PART_SET 2
#define RESOURCE_SET 3

#define REQUEST_SUCCESS 0
#define REQUEST_INVALID_SET 1
#define REQUEST_ID_COLLISION 2
#define REQUEST_ID_MISSING 3
#define REQUEST_ID_INVALID 4


short add_id(std::uint32_t id, short set);
short remove_id(std::uint32_t id, short set);
short create_id(std::uint32_t& id, short set);
short print_set(short set);

#endif