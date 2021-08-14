#ifndef ID_MANAGER_HPP
#define ID_MANAGER_HPP

#include <cstdint>

#include "common.hpp"

#define VESSEL_SET 1
#define PART_SET 2
#define RESOURCE_SET 3 // not in use

#define REQUEST_SUCCESS 0
#define REQUEST_INVALID_SET 1
#define REQUEST_ID_COLLISION 2
#define REQUEST_ID_MISSING 3
#define REQUEST_ID_INVALID 4


/*
 * Functions used to manage object IDs. The IDs are stored in three sets, a vessel set, part set
 * and resource set. These functions are not thread safe, so these functions should be called from
 * the main thread. The ID 0 is a reserved ID and it's not valid. These functions may return
 * REQUEST_INVALID_SET if the picked set does not exist. For the vessel and part sets, these IDs
 * are supposed to be unique identifiers of a particular object (m_unique_id and m_vessel_id). The
 * resource set is not being used, the IDs are managed by the unique_map in the asset manager.
 */


/*
 * Adds the given ID to one of the sets. If the ID already exists, the function will return
 * REQUEST_ID_COLLISION. If the ID value is 0, the function returns REQUEST_ID_INVALID.
 *
 * @id: id to be added, should be a non-zero unique ID.
 * @set: set where the ID is going to be added.
 */
short add_id(std::uint32_t id, short set);

/*
 * Removes the given ID from the set, should be used when the object is destroyed. If the given ID
 * doesn't exist the function will return REQUEST_ID_MISSING.
 *
 * @id: ID to be removed.
 * @set: set where the ID is going to be removed from.
 */
short remove_id(std::uint32_t id, short set);

/*
 * Generates a random non-zero ID in a particular set.
 *
 * @id: reference where the ID is going to be stored.
 * @set: set where the ID is going to be generated.
 */
short create_id(std::uint32_t& id, short set);

/*
 * Prints all the IDs from the given set.
 */
short print_set(short set);

#endif