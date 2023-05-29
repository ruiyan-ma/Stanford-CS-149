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
    finished_(true),
    finished_level_(-1),
    counter_(0) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    threads_ = new std::thread[num_threads_];
    mutex_ = new std::mutex();
    condition_variable_ = new std::condition_variable();

    //work_queue = std::vector<std::vector<TaskID>>();
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
    //int curr_num_tasks_copy;
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

void TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked(int level, const std::vector<TaskID>& vec) {
    // this function will parallel the tasks on the same level in the DAG together
    int i = 0;
    int size = vec.size();
    int curr_num_tasks_copy, num_total_tasks;
    IRunnable* runnable;
    while (true) {
        if (finished_) return;
        mutex_->lock();
        if (id_finished.size() == size) {
            mutex_->unlock();
            finished_level_ = level;
            finished_ = true;
            condition_variable_->notify_all();
            return;               
        }
        auto tid = vec[i];
        if (id_finished.count(tid) == 0) {
            //mutex_->lock();
            curr_num_tasks_copy = id_curr_num_tasks[tid];
            num_total_tasks = id_num_tasks[tid];
            runnable = id_runnable[tid];
            id_curr_num_tasks[tid]++;
            //mutex_->unlock();
            // here we must use the copy local value, since the real value might has been changed by other threads
            if (curr_num_tasks_copy >= num_total_tasks) {
                id_finished.insert(tid);
                mutex_->unlock();
                i = (i + 1) % size;
                continue;
            }
            mutex_->unlock();
            runnable->runTask(curr_num_tasks_copy, num_total_tasks);
        } else mutex_->unlock();

        i = (i + 1) % size;
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {

                                                        
    //
    // TODO: CS149 students will implement this method in Part B.
    //

    // update the bookeeping info map
    mutex_->lock();
    id_runnable[counter_] = runnable;
    id_num_tasks[counter_] = num_total_tasks;
    mutex_->unlock();
    id_vecid[counter_] = deps; 

    // update the work queue
    work_queue.push_back({});
    // return the level that the current task belongs to
    std::cout << "prepare to update the DAG..." << std::endl;
    auto level = update_dag(deps, 0);

    //std::cout << "level: " << level << std::endl;
    work_queue[level].push_back(counter_);
    
    //id_level[counter_] = level;
    // if there is still work running, then we can directly return here
    if (!finished_) return counter_++;

    // // // if all the work has been finished, we join the threads
    join_threads();

    // remove the tasks that have been finished from the work_queue
    // 注意现在当前level的所有任务未必都完成，因为我们可能刚才上方的代码中在当前level加入了新的元素
    // 但finished_level代表了刚在处理的层
    if (finished_level_ >= 0) {
        if (id_finished.size() == work_queue[finished_level_].size()) {
            work_queue[finished_level_].clear();
        } else {
            std::cout << "current level: " << level << std::endl;
            std::cout << "finished level: " << finished_level_ << std::endl;
            std::cout << "size of finished set: " << id_finished.size() << std::endl;
            std::cout << "size of current level: " << work_queue[finished_level_].size() << std::endl;
            auto st = work_queue[finished_level_].begin();
            work_queue[finished_level_].erase(st, st + id_finished.size());
        }
        id_finished.clear();        
    }

    for (int level = 0; level < work_queue.size(); ++level) {
        if (!work_queue[level].empty()) {
            std::cout << "change level from " << finished_level_ << " to " << level - 1 << std::endl;
            finished_level_ = level - 1;
            break;
        }
    }

    // // so here we can assume all work int the above level has been finished now 
    // // find the 1st level which has not started yet
    runAsyncWithDepsHelper(finished_level_ + 1);
    return counter_++;
}

void TaskSystemParallelThreadPoolSleeping::runAsyncWithDepsHelper(int next_start_level) {

    // std::vector<TaskID> cur;
    // for (int level = 0; level < work_queue.size(); level++) {
    //     // if this level's work has been finished, continue
    //     if ((finished_level_ >> level) & 1) continue;
    //     for (auto & task : work_queue[level]) {
    //         mutex_map_->lock();
    //         if (id_curr_num_tasks[task] < id_num_tasks[task]) {
    //             // this task does not finished
    //             cur.push_back(task);
    //         }
    //         mutex_map_->unlock();
    //     }
    //     // give the unfinished work vector to the multithreading function
    //     if (cur.size()) {
    //         finished_ = false;
    //         for (int i = 0; i < num_threads_; i++) {
    //             threads_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked, this, level, cur);
    //         }
    //         break;
    //     }
    // } 

    // since this function will not be called at the same time with the multithreading function, mutex is not a must
    auto vec = work_queue[next_start_level];
    if (vec.empty()) return;
    finished_ = false;
    for (int i = 0; i < num_threads_; i++) {
        threads_[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::runTasksMultithreading_chunked, this, next_start_level, vec);
    }
}

int TaskSystemParallelThreadPoolSleeping::update_dag(const std::vector<TaskID>& deps, int depth) {
    int level = 0;
    //std::cout << "depth: " << depth << std::endl;
    for (auto &id : deps) {
        if (!id_depth.count(id)) {
            id_depth[id] = update_dag(id_vecid[id], depth + 1);
        }
        level = std::max(level, 1 + id_depth[id]);
    }
    return level;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
    // std::cout << "start to sync" << std::endl;
    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    
    // get the maxmium level
    int last_level = work_queue.size() - 1;
    for (; last_level > finished_level_ + 1; --last_level) {
        if (!work_queue[last_level].empty()) {
            break;
        }
    }    

    last_level += 1;

    std::unique_lock<std::mutex> lk(*mutex_);
    condition_variable_->wait(lk, [&]{return if_finished();});
    lk.unlock();
    join_threads();
    if (finished_level_ >= 0) {
        if (id_finished.size() == work_queue[finished_level_].size()) {
            work_queue[finished_level_].clear();
        } else {
            //std::cout << "current level: " << level << std::endl;
            std::cout << "finished level: " << finished_level_ << std::endl;
            std::cout << "size of finished set: " << id_finished.size() << std::endl;
            std::cout << "size of current level: " << work_queue[finished_level_].size() << std::endl;
            auto st = work_queue[finished_level_].begin();
            work_queue[finished_level_].erase(st, st + id_finished.size());
        }
        id_finished.clear();        
    }
    for (int level = 0; level < work_queue.size(); ++level) {
        if (!work_queue[level].empty()) {
            std::cout << "change level from " << finished_level_ << " to " << level - 1 << std::endl;
            finished_level_ = level - 1;
            break;
        }
    }
    //id_finished.clear(); 
    // in this loop, deal with one level per loop
    for (int i = finished_level_ + 1; i < last_level; ++i) {
        runAsyncWithDepsHelper(i);
        lk.lock();
        // block this thread and give away the CPU execution resources until the jon has been finished
        condition_variable_->wait(lk, [&]{return if_finished();});
        lk.unlock();
        join_threads();
        id_finished.clear();
    }
}
