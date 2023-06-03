# Notes on Assignment-2

本作业从零构造了一个线程池，并实现了其对不同任务的同步/异步执行机制。本作业使用的测试平台是`Ubuntu 20.04`，基于`Intel X86-64`架构。

程序的单线程版本由`TaskSystemSerial`实现。

## part_a (Sync)

本部分的线程池采用了三种不同机制实现。

### TaskSystemParallelSpawn

这部分的实现是最基础的多线程操作，每一次在函数`run()`的开始产生多线程，在`run()`末尾将生成的线程`join()`。这里我使用的是静态分配线程的方法，也即将大的`for`循环根据`i`的余数分割成几个不同的部分来给多个工作线程执行。

### TaskSystemParallelThreadPoolSpinning

从这一步开始，我们将实现一个线程池。当对象被创建（调用构造函数）的时候，线程池随即被创建（按照指定的线程数）：

```c++
for (int i = 0; i < num_threads; ++i) {
    threads_[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::runTaskMultiThreading, this);
}
```

`runTaskMultithreading`会一直执行，轮询是否有新的任务，主线程也会在调用`run()`之后一直等待（轮询）任务是否完成，直到任务被完成才返回。

而在对象被销毁的时候，线程才会被`join()`并结束：

```c++
for (int i = 0; i < num_threads_; i++)
    if (threads_[i].joinable()) threads_[i].join();
```

在这一步，工作线程以及主线程会一直`spin`来轮询任务是否完成/是否有新的任务加入工作队列。需要注意的是，我们切不可在`runTask()`执行时仍然持有互斥锁，这会让多线程失去意义。而为了保证`runTask()`调用时，给入参数的值不会因为没有持有锁而变成和预期不符的值，我们需要在还在`critical section`内部时，使用局部变量来保存当前`shared variable`的值，并在`runTask()`时给入局部变量的值，这样就确保了不会发生错误。

由于我们必须确保在`run()`返回时，所有的任务都已经被执行完毕，这要求我们必须在`runTask()`函数之后加上判定代码，所以我设置了一个变量`runTask_time_`用来记录当前`runTask()`已经被执行了多少次；为了增加系统并法度，这里是用了原子变量`std::atomic<T*>`。

关于原子变量以及无锁（`lock-free`）编程，其实有很多可以谈的，包括一些用来判定代码执行顺序的方法（`happen-before`, `synchronize-with`, `sequence-before`, `inter-thread happen-before`等），以及不同的内存顺序（`memory order`），这里不展开，可以参考网上很多文章，讲的很详细。由于这里只是对原子变量做了一个简单的自增（默认使用`std::memory_order_seq_cst`内存顺序），所以不存在其他问题。

### TaskSystemParallelThreadPoolSleeping

上述方法的问题在于：

- 对于主线程来说，他需要一直轮询检查工作线程是否完成任务
- 对于工作线程来说，他需要一直轮询来检查主线程是否给入了新的任务

所以我们想让线程将原本的轮询状态变成沉睡状态，这就需要用到条件变量（虽然本质上`mutex`也是一种`sleeplock`，但他不能做到在条件上等待）。所以给主线程和工作线程分别设置一个条件变量。

这里需要注意的一点是，C++中的`notify_all`函数会唤醒所有线程，此时如果满足对应的`lambda`条件，`wait`函数返回后线程会自动尝试获得对应的互斥锁，所以在同一时刻一定只能有一个工作线程继续运行，而其他没有拿到锁的线程就会继续沉睡，但这些沉睡的线程**已经从条件变量序列移到了互斥锁序列**，也就是说为了唤醒所有的线程，我们只需要一句`notify_all`就可以。

当然，如果在其余的线程获得锁之后检查发现条件不再为真，那就会再次进入条件变量序列，继续沉睡，这样的结果又被称为虚假唤醒（`spurious wakeup`），这也是为什么我们一般使用谓词`wait`的原因（相当于一层`while`循环）来放置虚假唤醒让条件不再为真时的线程继续运行。

**虚假唤醒**除了上述的这种由于代码执行逻辑而产生之外，还有可能来源于操作系统层面的`EINTR`信号，在这种情况下，我们并没有主动调用`notify/broadcast`，但是线程仍然被唤醒：`pthread` 的条件变量等待 `pthread_cond_wait` 是使用阻塞的系统调用实现的（比如 Linux 上的 `futex`），这些阻塞的系统调用在进程被信号中断后，通常会中止阻塞、直接返回 EINTR 错误。而将`wait`放入循环中也正可以避免这种情况对程序执行造成影响。

## part_b (Async)

这部分实现线程池的异步执行，具体来说就是当`run()`返回后，任务不一定执行完毕，只有当调用`sync()`并返回时，所有的任务才确保执行完成。

这一部分的另一难点在于处理多线程任务中的依赖，我们必须先确保被依赖的任务全部执行完成后，再执行后续的任务。

因为是异步执行，所以我们需要方法记录每一个任务对应的`Irunnable`对象，任务数量（`num_total_tasks`）以及所依赖的`deps`等。为了记录这些信息，我们可以使用STL提供的容器类型，但需需要注意STL是线程不安全的，所以在使用时需要加锁保证并发正确性。

```cpp
std::unordered_map<TaskID, IRunnable*> id_runnable;         // record the runnable corresponding with the TID
std::unordered_map<TaskID, int> id_num_tasks;               // record the num_total_tasks
std::unordered_map<TaskID, int> id_curr_num_tasks;          // record the task_ids that should be executed next
std::unordered_map<TaskID, std::vector<TaskID>> id_vecid;   // use this dependency vector to find the DAG
std::unordered_map<TaskID, int> id_depth;                   // record the depth/level of the task as memorization
std::vector<std::vector<TaskID>> work_queue;                // work queue that record dependencies of different level
```

### Asynchronous Task Launch

为了实现异步任务执行，主线程在`notify_all`所有子线程之后即可返回。同时准备一个工作队列，这里使用的是二维vector，每一个`vec[i]`代表第`i`层的任务。当`deps`数组不为空时，我们就叫当前的任务为`deps`数组中任务们的下一级任务，也就是说当前任务必须在`deps`中的任务全部执行完成后才能开始执行。

这里的设计允许`vec[i]`中的所有任务并行运行（也就是并行执行同一级的所有任务），所有工作线程都会循环递增（`idx=(idx+1)%size`）自己的局部变量`idx`并运行对应的`vec_[idx]`。所以为了判定当前这一级的任务是否全部被执行完成，我们统计当前层内需要执行任务的总次数，并在每一个`runTask()`后使用与`part_a`相同的方法递增一个原子计数`T`，如果`T`等于当前层内需要执行任务的总次数，说明所有任务均已完成。

需要注意的是，由于测试程序不一定完全按照从小到大的层级顺序给入任务，即使当前我们执行完了这一层的任务，仍然需要检查一下最新的工作队列中，前边的层级有无被主线程更新过（因为在多线程下工作线程解锁的时候，主线程可能往工作队列中前边的层级添加了新的任务）。

为了实现这个判定，每一次判定当前层级执行完毕后，都清空`work_queue[i].begin()`到`work_queue[i].begin() + vec_.size()`，并再次从第0级开始判定每一层的`vec[i]`是否为空，从第一个不为空的层级开始执行；如果当前工作队列中所有层级都是空的，那说明已经没有任务可以被执行，可以通知主线程，而工作线程可以继续沉睡。

### Support for Explicit Dependencies

前边提到，为了处理依赖，我们将工作队列设置成二维vector的形式。那如何判定加入的任务处在哪一级呢？我们需要遍历建立的**DAG**，而为了加速这一过程，我们可以在递归中使用记忆化搜索：

```cpp
// when main thread add new task to the DAG
auto level = update_dag(deps);
id_depth[counter_] = level;

// update the DAG
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
```

### 



