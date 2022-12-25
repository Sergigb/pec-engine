#ifndef TIMING_HPP
#define TIMING_HPP


struct logic_timing{
    double acc_logic_load, acc_logic_sleep;
    double avg_logic_load, avg_logic_sleep;
    int update_freq, ticks_since_last_update;

    logic_timing(){
        acc_logic_load = 0.;
        acc_logic_sleep = 0.;
        update_freq = 60;
        ticks_since_last_update = 0;
    }

    void update(double load, double sleep){
        acc_logic_load += load;
        acc_logic_sleep += sleep;
        ticks_since_last_update += 1;

        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            avg_logic_load = acc_logic_load / 60000.0;
            avg_logic_sleep = acc_logic_sleep / 60000.0;
            acc_logic_sleep = 0;
            acc_logic_load = 0;
        }
    }
};


struct physics_timing{
    double acc_phys_load, avg_phys_load;
    int update_freq, ticks_since_last_update;

    physics_timing(){
        acc_phys_load = 0.;
        update_freq = 60;
        ticks_since_last_update = 0;
    }

    void update(double load){
        acc_phys_load += load;
        ticks_since_last_update += 1;

        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            avg_phys_load = acc_phys_load / 60000.0;
            acc_phys_load = 0;
        }
    }
};

#endif