#include "tasksys.h"


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads), num_threads(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    // delete[] threads;
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    threads = new std::thread[num_threads];
    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(&TaskSystemParallelSpawn::runTaskBalanced, this, i, runnable, num_total_tasks);
    }

    // for (int i = 0; i < num_total_tasks; i++) {
    //     runnable->runTask(i, num_total_tasks);
    // }

    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }

    delete[] threads;
}

void TaskSystemParallelSpawn::runTaskBalanced(int thread_id, IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; ++i) {
        if (i % num_threads == thread_id) {
            runnable->runTask(i, num_total_tasks);
        }
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): 
    ITaskSystem(num_threads), 
    num_threads_(num_threads), 
    curr_num_tasks_(0), 
    runnable_(nullptr), 
    num_total_tasks_(0), 
    finished_(true), 
    end_(false),
    run_time_(0) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    threads_ = new std::thread[num_threads];
    mutex_ = new std::mutex();

    for (int i = 0; i < num_threads; ++i) {
        threads_[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::runTaskMultiThreading, this);
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {

    mutex_->lock();
    end_ = true;
    mutex_->unlock();

    for (int i = 0; i < num_threads_; i++)
        if (threads_[i].joinable()) threads_[i].join();

    delete[] threads_;
    delete mutex_;
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // for (int i = 0; i < num_total_tasks; i++) {
    //     runnable->runTask(i, num_total_tasks);
    // }

    // 上一个小节使用的是静态分配工作线程的方法，这里尝试使用动态分配工作线程
    std::unique_lock<std::mutex> lk(*mutex_);
    finished_ = false;
    // must clear the shared variable in case for the next use
    runnable_ = runnable;
    num_total_tasks_ = num_total_tasks;
    lk.unlock();

    while (true) {
        lk.lock();
        if (finished_) {
            curr_num_tasks_ = 0;
            run_time_ = 0;
            return;
        }
        lk.unlock();
    }
}

void TaskSystemParallelThreadPoolSpinning::runTaskMultiThreading() {
    int curr_num_tasks_copy = 0;
    while (true) {
        std::unique_lock<std::mutex> lk(*mutex_);
        
        if (end_) return;
        if (!runnable_) continue;
        
        auto r = runnable_;
        auto n = num_total_tasks_;

        if (run_time_ == n) {
            finished_ = true;
            runnable_ = nullptr;
            continue;
        }
        
        curr_num_tasks_copy = curr_num_tasks_;
        curr_num_tasks_++;
        if (curr_num_tasks_copy >= n) continue;
        lk.unlock();
        r->runTask(curr_num_tasks_copy, n);
        //lk.lock();
        run_time_++;
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): 
    ITaskSystem(num_threads), 
    num_threads_(num_threads), 
    curr_num_tasks_(0), 
    runTask_time_(0),
    task_n_(0),
    task_r_(nullptr),
    end_(false), 
    finished_(true) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    threads_ = new std::thread[num_threads_];
    mutex_ = new std::mutex();
    condition_variable_ = new std::condition_variable();
    condition_variable_main_thread_ = new std::condition_variable();

    for (int i = 0; i < num_threads_; ++i) {
        threads_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runTasksMultithreading, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    end_ = true;

    //std::cout << "start loop" << std::endl;

    // notify all the threads
    //while (finished_threads_ < num_threads_)
    condition_variable_->notify_all();

    

    for (int i = 0; i < num_threads_; ++i) {
        if (threads_[i].joinable()) threads_[i].join();
    }

    

    delete[] threads_;
    delete mutex_;
    delete condition_variable_;
    delete condition_variable_main_thread_;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    task_r_ = runnable;
    task_n_ = num_total_tasks;
    finished_ = false;

    // 注意notify_all会唤醒所有线程，而拿不到锁的线程会轮询以试图获得锁，而不是再次沉睡
    condition_variable_->notify_all();
    std::unique_lock<std::mutex> lk(*mutex_);
    condition_variable_main_thread_->wait(lk, [this]{return is_finished();});
    task_n_ = 0;
    runTask_time_ = 0;
    curr_num_tasks_ = 0;
}

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading() {
    int curr_num_tasks_copy;
    while (true) {
        if (end_) return;

        std::unique_lock<std::mutex> lk(*mutex_);
        condition_variable_->wait(lk, [this]{return next_round_or_end();});

        if (end_) return;

        auto r = task_r_;
        auto n = task_n_;

        // 将判断加在这里可以确保当主线程被通知可以不再阻塞时，所有的任务都已经执行完毕
        if (runTask_time_ == n) {
            finished_ = true;
            lk.unlock();
            condition_variable_main_thread_->notify_one();
            continue;
        }       

        // now re-acquire the lk automatically
        curr_num_tasks_copy = curr_num_tasks_;
        curr_num_tasks_++;
        lk.unlock();
        if (curr_num_tasks_copy >= n) continue;
        
        r->runTask(curr_num_tasks_copy, n);

        runTask_time_++;

        // 将if比较放在这里可能在原子变量下产生指令重排
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
