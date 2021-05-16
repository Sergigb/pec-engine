#include <iostream>
#include <functional>

#include <tinyxml2.h>

#include "load_resources.hpp"
#include "../common.hpp"
#include "../log.hpp"
#include "../../assets/Resource.hpp"


typedef std::unordered_map<std::uint32_t, std::unique_ptr<Resource>>::iterator ResourceIterator;

const char* current_file; // not thread safe


const tinyxml2::XMLElement* get_resource_element(const tinyxml2::XMLElement* resource, 
                                                 const char* element_name){
    const tinyxml2::XMLElement* element = resource->FirstChildElement(element_name);

    if(!element){
        std::cerr << "load_resources::create_resource: missing element \""<< element_name 
                  << "\" for resource element found in line " << resource->GetLineNum() << " (" 
                  << current_file << ")" << std::endl;
        log("load_resources::create_resource: missing element ", element_name, " for resource"
            " element found in line ", resource->GetLineNum(), " (", current_file, ")");
        return nullptr;
    }
    return element;
}


int get_double(const tinyxml2::XMLElement* resource, const char* element_name, double &value){
    const tinyxml2::XMLElement* element = get_resource_element(resource, element_name);

    if(element){
        if(element->QueryDoubleText(&value)){
            std::cerr << "load_resources::get_double: invalid float value for element with"
                          " name \"" << element_name << "\" in resource found in line " <<
                         resource->GetLineNum() << " (" << current_file << ")" << std::endl;
            log("load_resources::get_double: invalid float value for element with name \"",
                 element_name, "\" in resource found in line ", resource->GetLineNum(), " (",
                 current_file, ")");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}


int get_int(const tinyxml2::XMLElement* resource, const char* element_name, int &value){
    const tinyxml2::XMLElement* element = get_resource_element(resource, element_name);

    if(element){
        if(element->QueryIntText(&value)){
            std::cerr << "load_resources::get_int: invalid integer value for element with"
                          " name \"" << element_name << "\" in resource found in line " <<
                         resource->GetLineNum() << " (" << current_file << ")" << std::endl;
            log("load_resources::get_int: invalid integer value for element with name \"",
                 element_name, "\" in resource found in line ", resource->GetLineNum(), " (",
                 current_file, ")");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}


int get_string(const tinyxml2::XMLElement* resource, const char* element_name, 
               const char** string){
    const tinyxml2::XMLElement* element = get_resource_element(resource, element_name);

    if(element){
        *string = element->GetText();
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}


int create_resource(std::unordered_map<std::uint32_t, std::unique_ptr<Resource>>& resource_map,
                     const tinyxml2::XMLElement* resource){
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
                  "same name" << std::endl;
    }

    return EXIT_SUCCESS;
}


void load_resources(std::unordered_map<std::uint32_t, std::unique_ptr<Resource>>& resource_map,
                    const char* path){
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
        return;
    }

    tinyxml2::XMLElement* resource = root->FirstChildElement("resource");

    if(!resource){
        std::cerr << "load_resources::load_resources: resource file " << path << "has no "
                     "\"resource\" elements after the root" << std::endl;
        log("load_resources::load_resources: resource file ", path, "has no \"resource\" elements "
            "after the root");
        return;
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
    
}

