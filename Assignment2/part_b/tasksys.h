#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <atomic>
#include <memory>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
        bool if_finished() {
            return finished_;
        };

        bool next_round_or_end() {
            return end_ || !finished_;
        };

    private:
        int num_threads_;
        // int curr_num_tasks_;
        std::thread* threads_;
        std::mutex* mutex_;
        std::condition_variable* condition_variable_;
        std::condition_variable* condition_variable_main_thread_;
        bool finished_;
        bool end_;

        std::atomic<int> sum_of_level_tasks_;

        //int next_start_level_;
        std::vector<TaskID> vec_;

        // this variable will always record the smallest level that is not finished
        int finished_level_;

        void runTasksMultithreading(TaskID tid, IRunnable* runnable, int num_total_tasks);
        void runTasksMultithreading_chunked();
        int update_dag(const std::vector<TaskID>& deps);

        TaskID counter_;

        std::unordered_map<TaskID, IRunnable*> id_runnable;
        std::unordered_map<TaskID, int> id_num_tasks;
        std::unordered_map<TaskID, int> id_curr_num_tasks;
        std::unordered_map<TaskID, std::vector<TaskID>> id_vecid;   // use this to find the DAG
        std::unordered_map<TaskID, int> id_depth;                   // record the depth/level of the task
        std::vector<std::vector<TaskID>> work_queue;
        std::unordered_set<TaskID> id_finished;

        //int level_finished_;

        void join_threads() {
            for (int i = 0; i < num_threads_; ++i) {
                if (threads_[i].joinable()) threads_[i].join();
            }
        };

        void check_next_level();



        // the following is for sync execution
        int curr_num_tasks_;
        std::atomic<int> runTask_time_;
        int task_n_;
        IRunnable* task_r_;
};

#endif
