- 为什么多线程的加速在这里呈现非线性变化？

**负载不均衡**。观察到当只有三个线程时，中间的线程（对应`view1`中的高亮部分）需要比其他两个线程多得多的计算（迭代次数），我们在函数`workerThreadStart()`中添加的时间计算代码也验证了我们的这一猜想。

- 如何解决这一问题？

尽量让每一个线程有相同的计算负载：

```cpp
    for (size_t i = args->threadId; i < args->height; i += args->numThreads) {
        mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, 
        args->width, args->height, 
        i, 1, args->maxIterations, args->output);
    }
```

经过如上的改进后，加速基本呈线性关系。

- 当继续增大线程数量后发生什么？

我的机器是12核CPU，在线程数量增加到12时，如果再继续增大线程数量，加速反而会下降，这是因为当前的线程分配已经将所有CPU核（包括superscalar）分配出去，此后过多的线程会让OS进行太多的`context switch`，反而导致加速变缓。