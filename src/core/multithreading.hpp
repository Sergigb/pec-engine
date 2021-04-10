#ifndef MULTITHREADING_HPP
#define MULTITHREADING_HPP

#include <thread>
#include <mutex>
#include <condition_variable>


struct thread_monitor{
    std::mutex mtx_start;
    std::mutex mtx_end;
    std::condition_variable cv_start;
    std::condition_variable cv_end;
    bool worker_start = false;
    bool worker_ended = false;
};


#endif