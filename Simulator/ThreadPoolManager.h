#pragma once

#define THREADPOOL__
#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>
#include<vector>
#include<optional>
#include<functional>
#include<chrono>


//-----------------------helper classes for named arguments------------------------------
template<typename T>
class Named {
private:
    T value;
public:
    explicit Named(T value): value{value} {}
    operator T() const {return value;}
};

class NumTasks: public Named<int> {
    using Named<int>::Named;
};

class IterationsPerTask: public Named<int> {
    using Named<int>::Named;
};

class NumThreads: public Named<int> {
    using Named<int>::Named;
};


//-----------------------------------------------------------------------------------------
template<typename Producer>
class ThreadPoolManager {
private:
    Producer* producer_; //A copy of the producer object
    const int numThreads = -1; //Number of worker threads the pool will spawn
                               //(immutable after construction)
                               // Initialized to -1 here, set properly in the ctor
    std::vector<std::thread> workers;
    std::atomic_bool running = false; //Guards start() so it only actually starts once
    std::atomic_bool stopped = false; //Flag to ask workers to stop gracefully
    static thread_local int num_tasks_finished; //Per‑thread counter of how many tasks that thread ran
    											//thread_local makes each thread have its own independent int
    std::atomic_int total_num_tasks_finished { 0 };
    void worker_function(); //The function each worker thread runs
public:
    // take by reference, store pointer
    ThreadPoolManager(Producer& producer, NumThreads numThreads);

    bool start(); //spawn worker threads (only once)
    void stop_gracefully(); //set stop flag and join threads
    void wait_till_finish(); //join all threads (blocks until they’re done)
};

template<typename Producer>
void ThreadPoolManager<Producer>::worker_function() {
    while(!stopped) {
        auto task = producer_->getTask();
        if(!task) break;
        (*task)();
        ++num_tasks_finished;
        ++total_num_tasks_finished;
    }
    if(stopped) {
        std::cout << std::this_thread::get_id() << " - stopped gracefully after processing " << num_tasks_finished << " tasks" << std::endl;
    }
    else {
        std::cout << std::this_thread::get_id() << " - finished after processing " << num_tasks_finished << " tasks" << std::endl;
    }
}

template<typename Producer>
ThreadPoolManager<Producer>::ThreadPoolManager(Producer& producer, NumThreads numThreads)
    : producer_(std::move(producer)),
      numThreads(numThreads)
	{
      workers.reserve(numThreads);
    }

template<typename Producer>
bool ThreadPoolManager<Producer>::start() {
        bool running_status = false;
        if(!running.compare_exchange_strong(running_status, true)) {
            return false;
        }
        for(int i=0; i<numThreads; ++i) {
            workers.push_back(std::thread([this]{
                worker_function();
            }));
        }
        return true;
    }

template<typename Producer>
void ThreadPoolManager<Producer>::stop_gracefully() {
    stopped = true;
    wait_till_finish();
}

template<typename Producer>
void ThreadPoolManager<Producer>::wait_till_finish() {
    for(auto& t : workers) {
        t.join();
    }
    std::cout << "thread pool finished/stopped after processing " << total_num_tasks_finished << " task(s)" << std::endl;
}


template<typename Producer>
thread_local int ThreadPoolManager<Producer>::num_tasks_finished { 0 };

