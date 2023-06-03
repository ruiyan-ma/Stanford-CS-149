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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

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

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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
    finished_(true),
    finished_level_(-1),
    counter_(0),
    end_(false),
    sum_of_level_tasks_(0),
    curr_num_tasks_(0),
    runTask_time_(0),
    task_n_(0),
    task_r_(nullptr) {
    //level_finished_(0) {
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

    // create a thread pool
    for (int i = 0; i < num_threads_; ++i) {
        threads_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    //std::cout << "start delete" << std::endl;
    mutex_->lock();
    end_ = true;
    mutex_->unlock();

    condition_variable_->notify_all();
    //std::cout << "loop finished" << std::endl;
    join_threads();
    //std::cout << "finish delete" << std::endl;

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

    task_r_ = runnable;
    task_n_ = num_total_tasks;
    finished_ = false;

    // 注意notify_all会唤醒所有线程，而拿不到锁的线程会轮询以试图获得锁，而不是再次沉睡
    condition_variable_->notify_all();
    std::unique_lock<std::mutex> lk(*mutex_);
    condition_variable_main_thread_->wait(lk, [this]{return if_finished();});
    task_n_ = 0;
    runTask_time_ = 0;
    curr_num_tasks_ = 0;
}

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading(TaskID tid, IRunnable* runnable, int num_total_tasks) {
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
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {

                                                        
    //
    // TODO: CS149 students will implement this method in Part B.
    //
    std::unique_lock<std::mutex> lk(*mutex_);

    // update the bookeeping info map
    id_runnable[counter_] = runnable;
    id_num_tasks[counter_] = num_total_tasks;
    lk.unlock();
    id_vecid[counter_] = deps; 

    // return the level that the current task belongs to
    auto level = update_dag(deps);
    id_depth[counter_] = level;

    lk.lock();
    // update the work queue
    work_queue.emplace_back();
    //std::cout << "level: " << level << std::endl;
    work_queue[level].push_back(counter_);
    lk.unlock();
    condition_variable_->notify_all();

    return counter_++;
}

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked() {
    // this function will parallel the tasks on the same level in the DAG together
    int i = 0;
    
    int curr_num_tasks_copy, num_total_tasks;
    IRunnable* runnable;
    while (true) {

        // ---critical section---
        std::unique_lock<std::mutex> lk(*mutex_);

        condition_variable_->wait(lk, [this]{return next_round_or_end();});

        if (end_) return;

        int size = vec_.size();

        int sum = 0;

        for (auto & v : vec_) {
            sum += id_num_tasks[v];
        }

        // 想要确保该分支只进入一次
        if (sum == sum_of_level_tasks_) {
            finished_level_++;
            finished_ = true;
            sum_of_level_tasks_ = 0;

            check_next_level();
            
            // if the finished_ is still true after executing the checking function, then there is no more in the queue
            if (finished_) {
                lk.unlock();
                condition_variable_main_thread_->notify_one(); 
                continue;                    
            }        
        }

        auto tid = vec_[i];
        id_finished.insert(tid);

        curr_num_tasks_copy = id_curr_num_tasks[tid];
        num_total_tasks = id_num_tasks[tid];
        runnable = id_runnable[tid];
        id_curr_num_tasks[tid]++;
        lk.unlock();

        i = (i + 1) % size;

        // here we must use the copy local value, since the real value might has been changed by other threads
        if (curr_num_tasks_copy >= num_total_tasks) continue;

        runnable->runTask(curr_num_tasks_copy, num_total_tasks);

        sum_of_level_tasks_++;    
    }
}


// This function must be used under the protection of lock
void TaskSystemParallelThreadPoolSleeping::check_next_level() {

    auto st = work_queue[finished_level_].begin();
    work_queue[finished_level_].erase(st, st + id_finished.size());   
    id_finished.clear();     

    for (size_t level = 0; level < work_queue.size(); ++level) {
        if (!work_queue[level].empty()) {
            finished_level_ = level - 1;
            break;
        }
    }

    if (finished_level_ + 1 < work_queue.size()) {
        vec_ = work_queue[finished_level_ + 1];
        if (vec_.size()) {
            finished_ = false;
        }
    }
}

int TaskSystemParallelThreadPoolSleeping::update_dag(const std::vector<TaskID>& deps) {
    int level = 0;
    for (auto &id : deps) {
        //std::cout << "update id: " << id << std::endl;
        if (!id_depth.count(id)) {
            id_depth[id] = update_dag(id_vecid[id]);
        }
        level = std::max(level, 1 + id_depth[id]);
    }
    return level;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
    //std::cout << "start to sync" << std::endl;
    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    std::unique_lock<std::mutex> lk(*mutex_);
    condition_variable_main_thread_->wait(lk, [this]{return if_finished();});
    //lk.unlock();
    
    
    // get the maxmium level
    check_next_level();

    if (finished_) return;

    lk.unlock();
    condition_variable_->notify_all();

    lk.lock();

    condition_variable_main_thread_->wait(lk, [this]{return if_finished();});
}
