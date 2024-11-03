#include <iostream>
#include <functional>

#include <tinyxml2.h>

#include "load_resources.hpp"
#include "xml_utils.hpp"
#include "../common.hpp"
#include "../log.hpp"
#include "../../assets/Resource.hpp"


typedef ResourceMap::iterator ResourceIterator;

const char* current_file; // not thread safe


int create_resource(ResourceMap& resource_map, const tinyxml2::XMLElement* resource){
    std::pair<ResourceIterator, bool> res;
    std::unique_ptr<Resource> resource_uptr;
    std::hash<std::string> str_hash; // size_t = 64bit?? change???

    const char* name,* fancy_name;
    int resource_type, resource_state;
    double density, temperature;

    if(get_string(resource, "name", &name) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if(get_string(resource, "fancy_name", &fancy_name) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if(get_int(resource, "type", resource_type) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if(get_int(resource, "state", resource_state) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if(get_double(resource, "density", density) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if(get_double(resource, "temperature", temperature) == EXIT_FAILURE)
        return EXIT_FAILURE;

    resource_uptr.reset(new Resource(name, std::string(fancy_name), resource_type,
                                     resource_state, density, temperature));
    resource_uptr->setId(str_hash(name));
    res = resource_map.insert({str_hash(name), std::move(resource_uptr)});

    if(!res.second){
        log("load_resources::create_resource: failed to insert resource with id ", 
            (std::uint32_t)str_hash(name), " (collided with another resource with the same name)");
        std::cerr << "load_resources::create_resource: failed to insert resource with id " 
                  << (std::uint32_t)str_hash(name) << " (collided with another resource with the "
                  "same name)" << std::endl;
    }
    log("load_resources::create_resource: created and inserted resource ", name," with id ", 
        (std::uint32_t)str_hash(name));

    return EXIT_SUCCESS;
}


int load_resources(ResourceMap& resource_map, const char* path){
    current_file = path;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* root;
    doc.LoadFile(path);

    if(doc.Error()){
        std::cerr << "load_resources::load_resources: LoadFile returned an error with code " 
                  << doc.ErrorID() << " for document " << path << std::endl;
        log("load_resources::load_resources: LoadFile returned an error with code ",
            doc.ErrorID(), " for document ", path);
    }

    root = doc.RootElement();

    if(!root){
        std::cerr << "load_resources::load_resources: can't get the root element for the document "
                  << path << std::endl;
        log("load_resources::load_resources: can't get the root element for the document ", path);
        return EXIT_FAILURE;
    }

    tinyxml2::XMLElement* resource = root->FirstChildElement("resource");

    if(!resource){
        std::cerr << "load_resources::load_resources: resource file " << path << "has no "
                     "\"resource\" elements after the root" << std::endl;
        log("load_resources::load_resources: resource file ", path, "has no \"resource\" elements "
            "after the root");
        return EXIT_FAILURE;
    }

    while(resource){
        if(create_resource(resource_map, resource) == EXIT_FAILURE){
            std::cerr << "load_resources::load_resources: failed to create resource found in line "
                      << resource->GetLineNum() << " for file " << path << std::endl;
            log("load_resources::load_resources: failed to create resource found in line ", 
                resource->GetLineNum(), " for file ", path);
        }
        resource = resource->NextSiblingElement("resource");
    }
    return EXIT_SUCCESS;
}

