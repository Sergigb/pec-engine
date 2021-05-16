#ifndef LOAD_RESOURCES_HPP
#define LOAD_RESOURCES_HPP

#include <memory>
#include <unordered_map>

class Resource;


void load_resources(std::unordered_map<std::uint32_t, std::unique_ptr<Resource>>& resource_map,
                    const char* path="../data/resources.xml");


#endif


