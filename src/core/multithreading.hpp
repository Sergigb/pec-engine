#ifndef MULTITHREADING_HPP
#define MULTITHREADING_HPP

#include <thread>
#include <mutex>
#include <condition_variable>


/*
 * Structure used to synchronize threads, there should be a master thread that wakes up the worker
 * thread, if the worker ends before the master it will notify the master and lock itself. If the
 * master has already finished, it will lock itself until the worker has ended, when the worker
 * ends it will notify the master.

 * Example usage of the worker waiting for the master to wake it up:
 *
 * std::unique_lock<std::mutex> lck(m_thread_monitor->mtx_start); // acquire the lock
 * while(!m_thread_monitor->worker_start){     // avoid spurious wakeups
 *     m_thread_monitor->cv_start.wait(lck);   // wait for cv_start.notify_all() from the master
 * }                                           // thread using the lock
 * m_thread_monitor->worker_start = false;     // set this var to false as we have started
 *
 * Example usage of the master waking up the worker
 *
 * std::unique_lock<std::mutex> lck2(m_thread_monitor.mtx_start); // acquire the lock
 * m_thread_monitor.worker_start = true;       // this is not a spurious wakeup
 * m_thread_monitor.cv_start.notify_all();     // notify the worker
 *
 * @mtx_start: mutex used at the start of the loop to lock cv_start.
 * @mtx_end: mutex used at the end of the loop to lock cv_end.
 * @cv_start: conditional variable used by the master thread to wake up the worker thread, the
 * worker will wait until this variable has changed.
 * @cv_end: conditional variable used by the worker thread to notify the master that it has 
 * finished.
 * @worker_start: variable used to avoid spurious wakeups by the worker while it's waiting for the
 * master.
 * @worker_ended: variable used to avoid spurious wakeups by the master while it's waiting for the
 * worker to end.
 */

struct thread_monitor{
    std::mutex mtx_start;
    std::mutex mtx_end;
    std::condition_variable cv_start;
    std::condition_variable cv_end;
    bool worker_start = false;
    bool worker_ended = false;
};


#endif