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
    //curr_num_tasks_(0), 
    //finished_threads_(0),
    counter_(0),
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
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    delete[] threads_;
    delete mutex_;
    delete condition_variable_;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_threads_; ++i) {
        threads_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runTasksMultithreading, this, counter_, runnable, num_total_tasks);
    }


    // std::unique_lock<std::mutex> lk(*mutex_);
    // condition_variable_->wait(lk, [&]{return if_finished();});

    // for (int i = 0; i < num_threads_; ++i) {
    //     threads_[i].join();
    // }

    // curr_num_tasks_ = 0;
    // finished_threads_ = 0;
}

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading(TaskID tid, IRunnable* runnable, int num_total_tasks) {
    int curr_num_tasks_copy;
    // while (true) {
    //     //mutex_->lock();
    //     // now re-acquire the lk automatically
    //     curr_num_tasks_copy = id_curr_num_tasks[tid];
    //     id_curr_num_tasks[tid]++;
    //     //mutex_->unlock();
    //     if (curr_num_tasks_copy >= num_total_tasks) {
    //         id_finished_threads[tid]++;
    //         if (id_finished_threads[tid] >= num_threads_) {
    //             finished_ = true;
    //         }
    //         condition_variable_->notify_one();
    //         return;
    //     }
    //     runnable->runTask(curr_num_tasks_copy, num_total_tasks);
    // }
}

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked(const std::vector<TaskID>& id_vec) {
    // this function will parallel the tasks on the same level in the DAG together
    int i = 0;
    int size = id_vec.size();
    std::unordered_set<TaskID> id_finished;
    std::unordered_map<TaskID, int> curr_num_tasks_copy;
    while (true) {
        auto tid = id_vec[i];
        if (id_finished.size() == size) {
            finished_ = true;
            condition_variable_->notify_one();
            return;               
        }
        if (id_finished.count(tid) == 0) {
            curr_num_tasks_copy[tid] = id_curr_num_tasks[tid];
            id_curr_num_tasks[tid]++;
            // here we must use the copy local value, since the real value might has been changed by other threads
            if (curr_num_tasks_copy[tid] >= id_num_tasks[tid]) {
                id_finished.insert(tid);
                i = (i + 1) % size;
                continue;
            }
            id_runnable[tid]->runTask(curr_num_tasks_copy[tid], id_num_tasks[tid]);            
        }

        i = (i + 1) % size;
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {

                                                        
    //
    // TODO: CS149 students will implement this method in Part B.
    //

    // update the bookeeping info map
    id_runnable[counter_] = runnable;
    id_num_tasks[counter_] = num_total_tasks;
    id_vecid[counter_] = deps; 

    // update the work queue
    work_queue.push_back({});
    // return the level that the current task belongs to
    auto level = update_dag(deps);
    work_queue[level].push_back(counter_);
    //id_level[counter_] = level;
    // if there is still work running, then we can directly return here
    if (!finished_) return counter_++;
    //std::cout << "join started" << std::endl;
    
    for (int i = 0; i < num_threads_; ++i) {
        if (threads_[i].joinable()) threads_[i].join();
    }

    //std::cout << "join finished" << std::endl;
    // so here we can assume all work has been finished now 
    runAsyncWithDepsHelper();
    return counter_++;
}

void TaskSystemParallelThreadPoolSleeping::runAsyncWithDepsHelper() {
    // deal with the task on the same level simultaneously, if there are multiple tasks existing on the same level,
    // then group them by two, multithreading two tasks once on the same level
    // start by checking if each task has finished, if all the tasks on this level have been finished, then we can move
    // to the next level
    // if any of the tasks on this level is not finished, then check whether we can allocate any worker threads to the task
    // 
    std::vector<TaskID> cur;
    for (auto & level_vec : work_queue) {
        for (auto & task : level_vec) {
            if (id_curr_num_tasks[task] < id_num_tasks[task]) {
                // this task does not finished
                cur.push_back(task);
            }
        }
        // give the unfinished work vector to the multithreading function
        if (cur.size()) {
            finished_ = false;
            for (int i = 0; i < num_threads_; i++) {
                threads_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked, this, cur);
            }
            break;
        }
    } 
    
}

int TaskSystemParallelThreadPoolSleeping::update_dag(const std::vector<TaskID>& deps) {
    int level = 0;
    for (auto &id : deps) {
        level = std::max(level, 1 + update_dag(id_vecid[id]));
    }
    return level;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    // get the maxmium level
    int level = 0;
    for (; level < work_queue.size(); ++level) {
        if (work_queue[level].empty()) {
            break;
        }
    }

    level -= 1;


    std::unique_lock<std::mutex> lk(*mutex_);
    // in this loop, deal with one level per loop
    for (int i = 0; i < level; ++i) {
        // block this thread and give away the CPU execution resources until the jon has been finished
        condition_variable_->wait(lk, [&]{return if_finished();});
        // re-acquire the lock
        runAsyncWithDepsHelper();
    }

    // join
    for (int i = 0; i < num_threads_; ++i) {
        threads_[i].join();
    }
}
