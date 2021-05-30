#include <iostream>

#include <tinyxml2.h>

#include "xml_utils.hpp"
#include "../log.hpp"


const tinyxml2::XMLElement* get_element(const tinyxml2::XMLElement* parent, 
                                        const char* element_name){
    const tinyxml2::XMLElement* element = parent->FirstChildElement(element_name);

    if(!element){
        std::cerr << "xml_utils::get_element: missing element \"" << element_name
                  << "\" in parent element \"" << parent->Name() << "\" found in line " << 
                  parent->GetLineNum() << std::endl;
        log("xml_utils::get_element: missing element \"", element_name, "\" in parent element ", 
            parent->Name(), " found in line ", parent->GetLineNum());
        return nullptr;
    }
    return element;
}


int get_double(const tinyxml2::XMLElement* parent, const char* element_name, double &value){
    const tinyxml2::XMLElement* element = get_element(parent, element_name);

    if(element){
        if(element->QueryDoubleText(&value)){
            std::cerr << "xml_utils::get_double: invalid float value for element with name \""
                         << element_name << "\" in parent element \"" << parent->Name() << "\""
                         " found in line " << parent->GetLineNum() << std::endl;
            log("xml_utils::get_double: invalid float value for element with name \"",
                element_name, "\" in parent element \"", parent->Name(), "\" found in line ",
                parent->GetLineNum());
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}


int get_int(const tinyxml2::XMLElement* parent, const char* element_name, int &value){
    const tinyxml2::XMLElement* element = get_element(parent, element_name);

    if(element){
        if(element->QueryIntText(&value)){
            std::cerr << "xml_utils::get_int: invalid integer value for element with name \""
                         << element_name << "\" in parent element \"" << parent->Name() << "\""
                         " found in line " << parent->GetLineNum() << std::endl;
            log("xml_utils::get_int: invalid integer value for element with name \"", element_name,
                "\" in parent element \"", parent->Name(), "\" found in line ", 
                parent->GetLineNum());
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}


int get_string(const tinyxml2::XMLElement* parent, const char* element_name, 
               const char** string){
    const tinyxml2::XMLElement* element = get_element(parent, element_name);

    if(element){
        *string = element->GetText();
        return EXIT_SUCCESS;
    }
    else{
        return EXIT_FAILURE;
    }
}


int get_attribute(const tinyxml2::XMLElement* element, const char* att_name, const char** string){
    *string = element->Attribute(att_name);

    if(!*string){
        std::cerr << "xml_utils::get_attribute: missing attribute with name \"" << att_name <<
                     "\" for element with name \"" << element->Name() << "\" found in line " <<
                     element->GetLineNum() << std::endl;
        log("xml_utils::get_attribute: missing attribute with name \"", att_name, "\" for element "
            "with name \"", element->Name(), " found in line ", element->GetLineNum());

        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}



