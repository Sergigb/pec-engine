#include<iostream>

#include <tinyxml2.h>

#include "load_parts.hpp"
#include "xml_utils.hpp"
#include "../log.hpp"
#include "../RenderContext.hpp"
#include "../BaseApp.hpp"
#include "../Physics.hpp"
#include "../../assets/Model.hpp"
#include "../../assets/parts/parts.hpp"


/*
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLElement* system_element,* star_element,* planets_element;
    const char* system_name;
    doc.LoadFile(path);
    struct star system_star;
    std::unique_ptr<planet_map> planets(new planet_map());

    if(doc.Error()){
        std::cerr << "load_star_system::load_star_system: LoadFile returned an error with code " 
                  << doc.ErrorID() << " for document " << path << std::endl;
        log("load_resources::load_resources: LoadFile returned an error with code ",
            doc.ErrorID(), " for document ", path);
        return EXIT_FAILURE;
    }

    system_element = doc.RootElement();





#include "assets_utils.hpp"
#include "../RenderContext.hpp"
#include "../Physics.hpp"
#include "../log.hpp"
#include "../../assets/parts/parts.hpp"
#include "../../assets/Model.hpp"






*/
/*
void get_instance_by_name(const char* class_name, Model* model, Physics* physics,
                          btCollisionShape* col_shape, btScalar dry_mass,
                          AssetManagerInterface* asset_manager, ){

    std::hash<std::string> str_hash; // :)

    switch(str_hash(class_name)){
        case str_hash("Separator"):
            Separator(model, physics, col_shape, dry_mass, str_hash("Separator"), asset_manager) : 
            return
        case str_hash("BasePart"):
            Separator(model, physics, col_shape, dry_mass, str_hash("BasePart"), asset_manager) : 

        default:

    }

}

*/

int load_parts(BasePartMap& part_map, const char* path, BaseApp* app){
    tinyxml2::XMLDocument doc;
    doc.LoadFile(path);
    //const char* name, *path;
    //std::pair<planet_map::iterator, bool> res;
    std::hash<std::string> str_hash;
    AssetManager* asset_manager = app->getAssetManager();

    if(doc.Error()){
        std::cerr << "load_parts::load_parts: LoadFile returned an error with code " 
                  << doc.ErrorID() << " for document " << path << std::endl;
        log("load_parts::load_parts: LoadFile returned an error with code ",
            doc.ErrorID(), " for document ", path);
        return EXIT_FAILURE;
    }

    const tinyxml2::XMLElement* part_element = doc.FirstChildElement("part");

    if(!part_element)
        return EXIT_FAILURE;

    while(part_element){

        const char* part_class, *part_name;

        if(get_string(part_element, "part_class", &part_class) == EXIT_FAILURE)
            return EXIT_FAILURE;

        if(get_string(part_element, "part_name", &part_name) == EXIT_FAILURE)
            return EXIT_FAILURE;

        /// coll shape
        int collision_shape;
        const tinyxml2::XMLElement* collision_element = 
            get_element(part_element, "collision");

        if(!collision_element){
            std::cerr << "Missing collision element for part defined in line " 
                      << part_element->GetLineNum() << std::endl;
            log("Missing collision element for part defined in line ", 
                part_element->GetLineNum());

            return EXIT_FAILURE;
        }

        if(get_int(collision_element, "shape", collision_shape) == EXIT_FAILURE)
            return EXIT_FAILURE;

        const tinyxml2::XMLElement* properties_element = 
            get_element(collision_element, "properties");

        if(!properties_element){
            std::cerr << "Missing properties element for collision element defined in line " 
                      << collision_element->GetLineNum() << std::endl;
            log("Missing properties element for collision element defined in line ",
                collision_element->GetLineNum());

            return EXIT_FAILURE;
        }

        std::unique_ptr<btCollisionShape> shape;
        switch(collision_shape){
            case CYLINDER_SHAPE:
                double param1, param2, param3;

                if(get_double(properties_element, "param1", param1))
                    return EXIT_FAILURE;
                if(get_double(properties_element, "param2", param2))
                    return EXIT_FAILURE;
                if(get_double(properties_element, "param3", param3))
                    return EXIT_FAILURE;

                shape.reset(new btCylinderShape(btVector3(param1, param2, param3)));

                break;

                //return EXIT_SUCCESS;
            //case str_hash("Other"):
                //return

            default:
                std::cerr << "Invalid collision shape in collision element defined in line "
                          << collision_element->GetLineNum() << std::endl;
                log("Invalid collision shape in collision element defined in line ",
                    collision_element->GetLineNum());
                return EXIT_FAILURE;
        }

        // model
        const char* model_path;

        if(get_string(part_element, "model_path", &model_path) == EXIT_FAILURE){
            std::cerr << "Missing model path for part defined in line " 
                      << part_element->GetLineNum() << std::endl;
            log("Missing model path for part defined in line ", part_element->GetLineNum());

            return EXIT_FAILURE;
        }

        std::unique_ptr<Model> model(new Model(model_path, nullptr, SHADER_PHONG_BLINN_NO_TEXTURE,
                                     app->getFrustum(), app->getRenderContext(),
                                     math::vec3(0.75, 0.75, 0.75)));

        // mass
        double mass;
        if(get_double(part_element, "mass", mass) == EXIT_FAILURE)
            return EXIT_FAILURE;

        // hash class type and create part
        std::uint32_t base_id = str_hash(std::string(part_name));
        std::unique_ptr<BasePart> part;
        if(strcmp(part_class, "Separator") == 0){
            part.reset(new Separator(model.get(), app->getPhysics(),
                                     shape.get(), mass, base_id, asset_manager));
        }

        // other config? missing
        part->setColor(math::vec3(0.75, 0.75, 0.75));
        
        // HARDCODEED
        part->setParentAttachmentPoint(math::vec3(0.0, 0.065, 0.0), math::vec3(0.0, 0.0, 0.0));
        part->addAttachmentPoint(math::vec3(0.0, -0.065, 0.0), math::vec3(0.0, 0.0, 0.0));
        part->setName(std::string("separator_") + std::to_string(555));
        part->setFancyName("SeparatorCode");
        part->setCollisionGroup(CG_DEFAULT | CG_PART);
        part->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
        part->setProperties(PART_SEPARATES);
        
        // end part creation
        typedef BasePartMap::iterator map_iterator;
        std::pair<map_iterator, bool> res;
        res = part_map.insert({base_id, std::move(part)});

        if(!res.second){
            log("Failed to insert part with id ", base_id, " (collided with ",
                 res.first->first, ")");
            std::cerr << "Failed to insert part with id " << base_id << " (collided with "
                      << res.first->first << ")" << std::endl;
        }

        asset_manager->m_collision_shapes.push_back(std::move(shape));
        asset_manager->m_models.push_back(std::move(model));

        part_element = part_element->NextSiblingElement("part");        
    }

    return EXIT_SUCCESS;
}

