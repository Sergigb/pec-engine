#ifndef PLANET_HPP
#define PLANET_HPP


#include "../core/maths_funcs.hpp"


class RenderContext;


class Planet{
    private:
        dmath::mat4 m_planet_transform; // this will change
    public:
        Planet(RenderContext* render_context);
        ~Planet();

        void render(const dmath::vec3& cam_translation, const dmath::mat4 transform);
        void render(const dmath::vec3& cam_translation);

        dmath::mat4& getTransform();
        dmath::mat4 getTransform() const;
};


#endif
