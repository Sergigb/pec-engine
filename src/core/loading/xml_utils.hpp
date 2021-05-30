#ifndef XML_UTILS_HPP
#define XML_UTILS_HPP


namespace tinyxml2{
    class XMLElement;
}


int get_string(const tinyxml2::XMLElement* parent, const char* element_name, const char** string);

int get_int(const tinyxml2::XMLElement* parent, const char* element_name, int &value);

int get_double(const tinyxml2::XMLElement* parent, const char* element_name, double &value);

const tinyxml2::XMLElement* get_element(const tinyxml2::XMLElement* parent,
                                        const char* element_name);

int get_attribute(const tinyxml2::XMLElement* element, const char* att_name, const char** string);


#endif


