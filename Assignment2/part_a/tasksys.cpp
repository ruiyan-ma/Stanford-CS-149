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
    ITaskSystem(num_threads), num_threads_(num_threads), curr_num_tasks_(0) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    threads_ = new std::thread[num_threads];
    mutex_ = new std::mutex();
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
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
    for (int i = 0; i < num_threads_; ++i) {
        threads_[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::runTaskMultiThreading, this, runnable, num_total_tasks);
    }

    for (int i = 0; i < num_threads_; ++i) {
        threads_[i].join();
    }

    // must clear the shared variable in case for the next use
    curr_num_tasks_ = 0;
}

void TaskSystemParallelThreadPoolSpinning::runTaskMultiThreading(IRunnable* runnable, int num_total_tasks) {
    int curr_num_tasks_copy;
    while (true) {
        mutex_->lock();
        curr_num_tasks_copy = curr_num_tasks_;
        curr_num_tasks_++;
        mutex_->unlock();
        if (curr_num_tasks_copy >= num_total_tasks) {
            return;
        }
        runnable->runTask(curr_num_tasks_copy, num_total_tasks);
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
    mutex_main_thread_ = new std::mutex();
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

    // notify all the threads
    condition_variable_->notify_all();

    for (int i = 0; i < num_threads_; ++i) {
        if (threads_[i].joinable()) threads_[i].join();
    }

    delete[] threads_;
    delete mutex_;
    delete mutex_main_thread_;
    delete condition_variable_;
    delete condition_variable_main_thread_;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    task_r_ = runnable;
    task_n_ = num_total_tasks;
    // 只要在这条语句之后，自线程就可以开始运行了
    // 所以必须保证在更新false之前，task已经被更新
    finished_ = false;

    // 注意notify_all会唤醒所有线程，而拿不到锁的线程会轮询以试图获得锁，而不是再次沉睡
    condition_variable_->notify_all();
    std::unique_lock<std::mutex> lk(*mutex_main_thread_);
    condition_variable_main_thread_->wait(lk, [&]{return is_finished();});
    runTask_time_ = 0;
    curr_num_tasks_ = 0;
}

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading() {
    int curr_num_tasks_copy;
    while (true) {
        if (end_) return;

        std::unique_lock<std::mutex> lk(*mutex_);
        condition_variable_->wait(lk, [&]{return next_round_or_end();});

        if (end_) return;

        // now re-acquire the lk automatically
        curr_num_tasks_copy = curr_num_tasks_;
        curr_num_tasks_++;
        lk.unlock();
        if (curr_num_tasks_copy >= task_n_) continue;
        
        task_r_->runTask(curr_num_tasks_copy, task_n_);
        runTask_time_++;
        // 将判断加在这里可以确保当主线程被通知可以不再阻塞时，所有的任务都已经执行完毕
        if (runTask_time_ == task_n_) {
            finished_ = true;
            condition_variable_main_thread_->notify_one();
        }
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
