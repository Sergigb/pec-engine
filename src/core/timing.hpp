#ifndef TIMING_HPP
#define TIMING_HPP

#include <iostream>
#include <chrono>

#include "log.hpp"


typedef std::chrono::steady_clock::time_point time_point;
typedef std::chrono::duration<double, std::micro> duration;
#define sch_now std::chrono::steady_clock::now

#define TP_LOGIC_START 1
#define TP_LOGIC_END 2

struct logic_timing{
    time_point loop_start, loop_end;

    double acc_logic_load, acc_logic_sleep;
    double avg_logic_load, avg_logic_sleep;
    double delta_t, current_load, current_sleep;
    int update_freq, ticks_since_last_update;

    logic_timing(){
        delta_t = (1. / 60.) * 1000000.;
        acc_logic_load = 0.;
        acc_logic_sleep = 0.;
        avg_logic_load = 0.;
        avg_logic_sleep = 0.;
        update_freq = 60;
        ticks_since_last_update = 0;
    }


    void update(){
        current_load = duration(loop_end - loop_start).count();
        current_sleep = delta_t - current_load;

        acc_logic_load += current_load;
        acc_logic_sleep += current_sleep;
        ticks_since_last_update += 1;

        if(ticks_since_last_update == update_freq){
            ticks_since_last_update = 0;

            avg_logic_load = acc_logic_load / 60000.0;
            avg_logic_sleep = acc_logic_sleep / 60000.0;

            acc_logic_sleep = 0;
            acc_logic_load = 0;
        }
    }


    void register_tp(short tp){
        switch(tp){
        case TP_LOGIC_START:
            loop_start = sch_now();
            break;
        case TP_LOGIC_END:
            loop_end = sch_now();
            break;
        default:
            log("logic_timing::register_tp: invalid time point ", tp);
            std::cerr << "logic_timing::register_tp: invalid time point " << tp << std::endl;
        }
    }
};


#define TP_PHYSICS_START 1
#define TP_PHYSICS_END 2
#define TP_BULLET_END 3
#define TP_ORBIT_END 4
#define TP_GRAV_END 5

struct physics_timing{
    time_point loop_start, loop_end, end_bullet, end_orbital, end_gravity;
    double acc_phys_load, acc_kinematic, acc_orbital, acc_gravity, acc_bullet,
           avg_phys_load, avg_kinematic, avg_orbital, avg_gravity, avg_bullet;
    int update_freq, ticks_since_last_update;

    physics_timing(){
        acc_phys_load = 0.;
        acc_kinematic = 0.;
        acc_orbital = 0.;
        acc_gravity = 0.;
        acc_bullet = 0.;
        avg_phys_load = 0.;
        avg_kinematic = 0.;
        avg_orbital = 0.;
        avg_gravity = 0.;
        avg_bullet = 0.;
        update_freq = 60;
        ticks_since_last_update = 0;
    }


    void update(bool paused){
        acc_phys_load += duration(loop_end - loop_start).count();
        
        acc_gravity += paused ? 0.0 : duration(end_gravity - loop_start).count();
        acc_bullet += paused ? 0.0 : duration(end_bullet - end_gravity).count();
        acc_orbital += paused ? 0.0 : duration(end_orbital - end_bullet).count();
        acc_kinematic += paused ? 0.0 : duration(loop_end - end_orbital).count();
        ticks_since_last_update += 1;

        if(ticks_since_last_update == update_freq){
            ticks_since_last_update = 0;

            avg_phys_load = acc_phys_load / 60000.0;
            avg_kinematic = acc_kinematic / 60000.0;
            avg_orbital = acc_orbital / 60000.0;
            avg_gravity = acc_gravity / 60000.0;
            avg_bullet = acc_bullet / 60000.0;

            acc_phys_load = 0.0;
            acc_kinematic = 0.0;
            acc_orbital = 0.0;
            acc_gravity = 0.0;
            acc_bullet = 0.0;
        }
    }


    void register_tp(short tp){
        switch(tp){
        case TP_PHYSICS_START:
            loop_start = sch_now();
            break;
        case TP_PHYSICS_END:
            loop_end = sch_now();
            break;
        case TP_BULLET_END:
            end_bullet = sch_now();
            break;
        case TP_ORBIT_END:
            end_orbital = sch_now();
            break;
        case TP_GRAV_END:
            end_gravity = sch_now();
            break;
        default:
            log("physics_timing::register_tp: invalid time point ", tp);
            std::cerr << "physics_timing::register_tp: invalid time point " << tp << std::endl;
        }
    }
};


#define TP_RENDER_START 1
#define TP_RENDER_END 2
#define TP_SCENE_END 3
#define TP_GUI_END 4

struct render_timing{
    time_point loop_start, loop_end, end_scene, end_gui;
    double acc_rend_load, acc_scene, acc_gui, acc_imgui,
           avg_rend_load, avg_scene, avg_gui, avg_imgui;
    int update_freq, ticks_since_last_update;

    render_timing(){
        acc_rend_load = 0.;
        acc_scene = 0.;
        acc_gui = 0.;
        acc_imgui = 0.;
        avg_rend_load = 0.;
        avg_scene = 0.;
        avg_gui = 0.;
        avg_imgui = 0.;
        update_freq = 60;
        ticks_since_last_update = 0;
    }


    void update(){
        acc_rend_load += duration(loop_end - loop_start).count();
        acc_scene += duration(end_scene - loop_start).count();
        acc_gui += duration(end_gui - end_scene).count();
        acc_imgui += duration(loop_end - end_gui).count();

        ticks_since_last_update += 1;

        if(ticks_since_last_update == update_freq){
            ticks_since_last_update = 0;

            avg_rend_load = acc_rend_load / 60000.0;
            avg_scene = acc_scene / 60000.0;
            avg_gui = acc_gui / 60000.0;
            avg_imgui = acc_imgui / 60000.0;

            acc_rend_load = 0.0;
            acc_scene = 0.0;
            acc_gui = 0.0;
            acc_imgui = 0.0;
        }
    }


    void register_tp(short tp){
        switch(tp){
        case TP_RENDER_START:
            loop_start = sch_now();
            break;
        case TP_RENDER_END:
            loop_end = sch_now();
            break;
        case TP_GUI_END:
            end_gui = sch_now();
            break;
        case TP_SCENE_END:
            end_scene = sch_now();
            break;
        default:
            log("render_timing::register_tp: invalid time point ", tp);
            std::cerr << "render_timing::register_tp: invalid time point " << tp << std::endl;
        }
    }
};

#endif