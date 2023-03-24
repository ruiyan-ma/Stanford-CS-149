1. 使用`tasks`后的加速比为0.93，由于该函数中涉及大量的内存访问操作，所以使用`tasks`没有得到显著加速度提升的原因是`bandwidth`的限制。
2. 除了一次对X的读取，一次对Y的读取之外，我们要先把内存中的`result[i]`读到cache中，再向cache写入更新的`result[i]`内容后，将cache的内容写回内存中(除非使用`write-around`模式只会写回内存)，所以总共需要4次内存访问。
3. 这里涉及到`non-temporal memory access`的想法。

由于`cache`的`temporal`特性，每次我们使用（读或者写）一个内存位置时，CPU都会把对应的内存中的值放到`cache`中，因为大概率我们接下来还会利用这个值。

但是在这份代码中，对于每一个`result[i]`，我们只使用一次，所以将它先放到`cache`中没有任何意义，如果能够绕过`cache`，直接将等式右侧的计算结果写入内存中，就可以把`bandwidth`前的系数从4变为3.

而`intrinsics`确实提供了可以完成对应功能的函数：

```cpp
__m256i _mm256_stream_load_si256 (__m256i const* mem_addr)

void _mm256_stream_pd (double * mem_addr, __m256d a)

void _mm256_stream_ps (float * mem_addr, __m256 a)

void _mm256_stream_si256 (__m256i * mem_addr, __m256i a)

```





在本实验中，由于