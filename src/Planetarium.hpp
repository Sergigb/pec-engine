#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

#include "core/BaseApp.hpp"
#include "core/maths_funcs.hpp"


class FontAtlas;
class Text2D;
class Planet;
class PlanetariumGUI;


#define MAX_ITER 10
#define REAL_TIME_DT (1000. / 60.0) / 1000.0


/*
 * TODO: change to bullet math
 */
struct particle{
    dmath::vec3 origin;
    dmath::vec3 velocity;
    dmath::vec3 total_force;
    double mass;

    particle(const dmath::vec3& o, const dmath::vec3& v, btScalar m){
        origin = o;
        velocity = v;
        mass = m;
        total_force = dmath::vec3(0.0, 0.0, 0.0);
    }
    particle(const struct particle& p){
        origin = p.origin;
        velocity = p.velocity;
        mass = p.mass;
        total_force = dmath::vec3(0.0, 0.0, 0.0);
    }
};


class Planetarium : public BaseApp{
    private:
        double m_delta_t, m_seconds_since_j2000, m_dt_multiplier;
        std::unique_ptr<Text2D> m_text, m_text2;

        std::vector<const Planet*> m_bodies;
        uint m_pick;

        std::unique_ptr<PlanetariumGUI> m_planetarium_gui;

        std::vector<struct particle> m_particles;

        // temp vvvvv part of the GUI
        GLuint m_pred_vao, m_pred_vbo_vert, m_pred_vbo_ind;
        int m_predictor_steps;
        int m_predictor_period;

        void init();
        void logic();
        void updateSceneText();
        void initGl();
        void renderOrbits();
        void render();
        void renderParticles();
        void updatePredictionBuffer();
        void applyGravity();

        // solvers
        void solverExplicitEuler(struct particle& p, double delta_t);
        void solverSymplecticEuler(struct particle& p, double delta_t);

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void processInput();
    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};

#endif
