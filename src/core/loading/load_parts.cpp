#include <iostream>

#include <tinyxml2.h>

#include "load_parts.hpp"
#include "xml_utils.hpp"
#include "../log.hpp"
#include "../RenderContext.hpp"
#include "../BaseApp.hpp"
#include "../Physics.hpp"
#include "../../assets/Model.hpp"
#include "../../assets/parts/parts.hpp"


typedef tinyxml2::XMLElement xmle;


int load_collider(const xmle* part_element, std::unique_ptr<btCollisionShape>& shape){
    int collision_shape;
    const xmle* collision_element = get_element(part_element, "collision");

    if(!collision_element){
        std::cerr << "load_collider: Missing collision element for part defined in line "
                  << part_element->GetLineNum() << std::endl;
        log("load_collider: Missing collision element for part defined in line ", 
            part_element->GetLineNum());

        return EXIT_FAILURE;
    }

    if(get_int(collision_element, "shape", collision_shape) == EXIT_FAILURE)
        return EXIT_FAILURE;

    const xmle* properties_element = get_element(collision_element, "properties");

    if(!properties_element){
        std::cerr << "load_collider: Missing properties element for collision element defined in "
                     "line " << collision_element->GetLineNum() << std::endl;
        log("load_collider:Missing properties element for collision element defined in line ",
            collision_element->GetLineNum());

        return EXIT_FAILURE;
    }

    switch(collision_shape){
        case CYLINDER_SHAPE:{
            double param1, param2, param3;

            if(get_double(properties_element, "param1", param1))
                return EXIT_FAILURE;
            if(get_double(properties_element, "param2", param2))
                return EXIT_FAILURE;
            if(get_double(properties_element, "param3", param3))
                return EXIT_FAILURE;

            shape.reset(new btCylinderShape(btVector3(param1, param2, param3)));}

            break;

        case CONE_SHAPE:{
            double param1, param2;

            if(get_double(properties_element, "param1", param1))
                return EXIT_FAILURE;
            if(get_double(properties_element, "param2", param2))
                return EXIT_FAILURE;

            shape.reset(new btConeShape(param1, param2));}

            break;
        default:
            std::cerr << "load_collider: Invalid collision shape in collision element defined in"
                         " line " << collision_element->GetLineNum() << std::endl;
            log("load_collider: Invalid collision shape in collision element defined in line ",
                collision_element->GetLineNum());
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int load_model(const xmle* part_element, std::unique_ptr<Model>& model){
    const char* model_path;

    if(get_string(part_element, "model_path", &model_path) == EXIT_FAILURE){
        std::cerr << "load_model: Missing model path for part defined in line " 
                  << part_element->GetLineNum() << std::endl;
        log("load_model: Missing model path for part defined in line ",
            part_element->GetLineNum());

        return EXIT_FAILURE;
    }

    model.reset(new Model(model_path, nullptr, SHADER_PHONG_BLINN_NO_TEXTURE,
                math::vec3(0.75, 0.75, 0.75)));

    return EXIT_SUCCESS;
}


int set_parent_att_point(const xmle* part_element, std::unique_ptr<BasePart>& part){
    const xmle* p_att_element = get_element(part_element, "parent_att_point", true);
    if(p_att_element){
        const xmle* origin_element = get_element(p_att_element, "origin");
        const xmle* orient_element = get_element(p_att_element, "orientation");
        double x, y, z, ox, oy, oz;

        if(!origin_element){
            std::cerr << "set_parent_att_point: Missing origin element for parent attachment point"
                         " defined in line " << p_att_element->GetLineNum() << std::endl;
            log("set_parent_att_point: Missing origin element for parent attachment point defined "
                "in line ", p_att_element->GetLineNum());

            return EXIT_FAILURE;
        }

        if(!orient_element){
            std::cerr << "set_parent_att_point: Missing orientation element for parent attachment"
                         "point defined in line " << p_att_element->GetLineNum() << std::endl;
            log("set_parent_att_point: Missing orientation element for parent attachment point "
                "defined in line ", p_att_element->GetLineNum());

            return EXIT_FAILURE;
        }

        if(get_double(origin_element, "x", x))
            return EXIT_FAILURE;
        if(get_double(origin_element, "y", y))
            return EXIT_FAILURE;
        if(get_double(origin_element, "z", z))
            return EXIT_FAILURE;

        if(get_double(orient_element, "x", ox))
            return EXIT_FAILURE;
        if(get_double(orient_element, "y", oy))
            return EXIT_FAILURE;
        if(get_double(orient_element, "z", oz))
            return EXIT_FAILURE;

        part->setParentAttachmentPoint(math::vec3(x, y, z), math::vec3(ox, oy, oz));
    }
    return EXIT_SUCCESS;
}


int set_free_att_points(const xmle* part_element, std::unique_ptr<BasePart>& part){
    const xmle* att_pts_elem = get_element(part_element, "free_att_point", true);

    if(!att_pts_elem)
        return EXIT_SUCCESS;

    double x, y, z, ox, oy, oz;
    const xmle* origin_element = get_element(att_pts_elem, "origin");
    const xmle* orient_element = get_element(att_pts_elem, "orientation");

    if(!origin_element){
        std::cerr << "set_free_att_points: Missing origin element for free attachment"
                     "point defined in line " << att_pts_elem->GetLineNum() << std::endl;
        log("set_free_att_points: Missing origin element for free attachment point "
            "defined in line ", att_pts_elem->GetLineNum());

        return EXIT_FAILURE;
    }

    if(!orient_element){
        std::cerr << "set_free_att_points: Missing orientation element for free attachment"
                     " point defined in line " << att_pts_elem->GetLineNum() << std::endl;
        log("set_free_att_points: Missing orientation element for free attachment point"
            "defined in line ", att_pts_elem->GetLineNum());

        return EXIT_FAILURE;
    }

    if(get_double(origin_element, "x", x))
        return EXIT_FAILURE;
    if(get_double(origin_element, "y", y))
        return EXIT_FAILURE;
    if(get_double(origin_element, "z", z))
        return EXIT_FAILURE;

    if(get_double(orient_element, "x", ox))
        return EXIT_FAILURE;
    if(get_double(orient_element, "y", oy))
        return EXIT_FAILURE;
    if(get_double(orient_element, "z", oz))
        return EXIT_FAILURE;

    part->setFreeAttachmentPoint(math::vec3(x, y, z), math::vec3(ox, oy, oz));

    return EXIT_SUCCESS;
}


int set_att_points(const xmle* part_element, std::unique_ptr<BasePart>& part){
    const xmle* att_pts_elem = get_element(part_element, "att_points", true);

    if(att_pts_elem){
        const xmle* point_elem = att_pts_elem->FirstChildElement("point");
        
        while(point_elem){
            double x, y, z, ox, oy, oz;
            const xmle* origin_element = get_element(point_elem, "origin");
            const xmle* orient_element = get_element(point_elem, "orientation");

            if(!origin_element){
                std::cerr << "set_att_points: Missing origin element for attachment"
                             "point defined in line " << att_pts_elem->GetLineNum() << std::endl;
                log("set_att_points: Missing origin element for attachment point "
                    "defined in line ", att_pts_elem->GetLineNum());

                return EXIT_FAILURE;
            }

            if(!orient_element){
                std::cerr << "set_att_points: Missing orientation element for attachment"
                             " point defined in line " << att_pts_elem->GetLineNum() << std::endl;
                log("set_att_points: Missing orientation element for attachment point"
                    "defined in line ", att_pts_elem->GetLineNum());

                return EXIT_FAILURE;
            }

            if(get_double(origin_element, "x", x))
                return EXIT_FAILURE;
            if(get_double(origin_element, "y", y))
                return EXIT_FAILURE;
            if(get_double(origin_element, "z", z))
                return EXIT_FAILURE;

            if(get_double(orient_element, "x", ox))
                return EXIT_FAILURE;
            if(get_double(orient_element, "y", oy))
                return EXIT_FAILURE;
            if(get_double(orient_element, "z", oz))
                return EXIT_FAILURE;

            part->addAttachmentPoint(math::vec3(x, y, z), math::vec3(ox, oy, oz));
            
            point_elem = point_elem->NextSiblingElement("point");
        }
    }

    return EXIT_SUCCESS;
}


int set_properties(const xmle* part_element, std::unique_ptr<BasePart>& part){
    long properties = 0;
    const xmle* prop_element = get_element(part_element, "properties", true);

    if(prop_element){
        const xmle* property = prop_element->FirstChildElement("p");
        while(property){
            int prop;
            property->QueryIntText(&prop);
            properties |= prop;

            property = property->NextSiblingElement("p");
        }
    }
    part->setProperties(properties);

    return EXIT_SUCCESS;
}


int load_resources(const xmle* part_element, std::unique_ptr<BasePart>& part,
                   AssetManager* asset_manager){
    const xmle* resources_elem = get_element(part_element, "resources", true);
    std::hash<std::string> str_hash;

    if(resources_elem){
        const xmle* resource_elem = resources_elem->FirstChildElement("resource");

        while(resource_elem){
            const char* resource_name;
            double mass, max_mass;

            if(get_string(resource_elem, "name", &resource_name) == EXIT_FAILURE)
                return EXIT_FAILURE;
            if(get_double(resource_elem, "mass", mass) == EXIT_FAILURE)
                return EXIT_FAILURE;
            if(get_double(resource_elem, "max_mass", max_mass) == EXIT_FAILURE)
                return EXIT_FAILURE;

            try{
                part->addResource(
                    {asset_manager->m_resources.at(str_hash(resource_name)).get(),
                    (float)mass, (float)max_mass});
            }
            catch(const std::out_of_range &err){
                std::cerr << "load_resources: Invalid resource name in resource defined in line "
                          << resource_elem->GetLineNum() << " (" << err.what()
                          << ")" << std::endl;
                log("load_resources: Invalid resource name in resource defined in line ",
                    resource_elem->GetLineNum(), " (", err.what(), ")");
            }
            resource_elem = resource_elem->NextSiblingElement("resource");
        }
    }

    return EXIT_SUCCESS;
}


int load_parts(BasePartMap& part_map, const char* path, BaseApp* app){
    tinyxml2::XMLDocument doc;
    doc.LoadFile(path);
    std::hash<std::string> str_hash;
    AssetManager* asset_manager = app->getAssetManager();

    if(doc.Error()){
        std::cerr << "load_parts::load_parts: LoadFile returned an error with code " 
                  << doc.ErrorID() << " for document " << path << std::endl;
        log("load_parts::load_parts: LoadFile returned an error with code ",
            doc.ErrorID(), " for document ", path);
        return EXIT_FAILURE;
    }

    const xmle* part_element = doc.FirstChildElement("part");

    if(!part_element)
        return EXIT_FAILURE;

    while(part_element){
        const char *part_class, *part_name, *fancy_name;

        if(get_string(part_element, "part_class", &part_class) == EXIT_FAILURE)
            return EXIT_FAILURE;

        if(get_string(part_element, "part_name", &part_name) == EXIT_FAILURE)
            return EXIT_FAILURE;

        std::string part_name_str(part_name);

        if(get_string(part_element, "part_fancy_name", &fancy_name) == EXIT_FAILURE)
            return EXIT_FAILURE;

        // coll shape
        std::unique_ptr<btCollisionShape> shape;
        if(load_collider(part_element, shape) == EXIT_FAILURE)
            return EXIT_FAILURE;

        // model
        std::unique_ptr<Model> model;
        if(load_model(part_element, model) == EXIT_FAILURE)
            return EXIT_FAILURE;

        // mass
        double mass;
        if(get_double(part_element, "mass", mass) == EXIT_FAILURE)
            return EXIT_FAILURE;

        // hash class type and create part
        std::uint32_t base_id = str_hash(part_name_str);
        std::unique_ptr<BasePart> part;
        if(strcmp(part_class, "Separator") == 0){
            part.reset(new Separator(model.get(), app->getPhysics(),
                                     shape.get(), mass, base_id, asset_manager));
        }
        if(strcmp(part_class, "BasePart") == 0){
            part.reset(new BasePart(model.get(), app->getPhysics(),
                                    shape.get(), mass, base_id, asset_manager));
        }
        if(strcmp(part_class, "GenericEngine") == 0){
            part.reset(new GenericEngine(model.get(), app->getPhysics(),
                                         shape.get(), mass, base_id, asset_manager));
        }
        if(strcmp(part_class, "VegaSolidEngine") == 0){
            std::unique_ptr<VegaSolidEngine> vega_part(new VegaSolidEngine(model.get(),
                                                           app->getPhysics(), shape.get(),
                                                           mass, base_id, asset_manager));

            if(vega_part->loadCustom(part_element) == EXIT_FAILURE)
                return EXIT_FAILURE;

            part.reset(vega_part.release());
        }

        part->setColor(math::vec3(0.75, 0.75, 0.75));
        part->setName(part_name_str + std::to_string(base_id));
        part->setFancyName(fancy_name);
        part->setCollisionGroup(CG_DEFAULT | CG_PART);
        part->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

        // parent att point
        if(set_parent_att_point(part_element, part) == EXIT_FAILURE)
            return EXIT_FAILURE;

        // att points
        if(set_att_points(part_element, part))
            return EXIT_FAILURE;

        // free att points
        if(set_free_att_points(part_element, part))
            return EXIT_FAILURE;

        // properties
        if(set_properties(part_element, part))
            return EXIT_FAILURE;

        // resources
        if(load_resources(part_element, part, asset_manager))
            return EXIT_FAILURE;

        // end part creation
        std::string final_part_name;
        part->getName(final_part_name);
        log("load_parts: successfully created part with name ", final_part_name, " (",
            fancy_name, ") with id ", base_id);

        typedef BasePartMap::iterator map_iterator;
        std::pair<map_iterator, bool> res;
        res = part_map.insert({base_id, std::move(part)});

        if(!res.second){
            log("load_parts: Failed to insert part with id ", base_id, " (collided with ",
                 res.first->first, ")");
            std::cerr << "load_parts: Failed to insert part with id " << base_id 
                      << " (collided with " << res.first->first << ")" << std::endl;
        }
        log("load_parts: inserted part with base ID ", base_id);

        asset_manager->m_collision_shapes.push_back(std::move(shape));
        asset_manager->m_models.push_back(std::move(model));

        part_element = part_element->NextSiblingElement("part");    
    }

    return EXIT_SUCCESS;
}

