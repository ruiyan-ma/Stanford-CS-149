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

但使用这些指令会带来另一个问题，就是它们本身是要求内存对齐的，为了达到这一目的，我们需要做如下操作：

- 在`CXXFLAGS`中设定`-std=c++17`

- 在`heap`上分配内存对齐的数组

关于第二步，涉及到C++里的内存对齐的相关操作，这里总结一下。

##### 什么是内存对齐

元素放置的位置一定会在自己宽度的整数倍上开始。

对于基本类型：这没什么好说的，完全按照内存对齐的定义来理解即可。

对于结构体或者类：

```cpp
struct Foo {
  char c;
  int i;
  int j;
  long k;
};

// sizeof(Foo)=24, alignof(Foo)=8
```

首先，这里的`alignof`关键字在`C++11`中被引入，计算当前类型的对齐要求是多少。这里的`alignof(Foo)=8`很好理解，因为结构体内部的对齐需要保证所有元素都在自己宽度的整数倍上开始就必须挑选最大的那一个。

那么结构体为了确保对齐需求，就得在比对齐需求小的元素后方做内存填充：

```cpp
/* Offset of member MEMBER in a struct of type TYPE. */
#define offsetof(OBJECT_TYPE, MEMBER) __builtin_offsetof (OBJECT_TYPE, MEMBER)

// ...test code...

// 0   # c 的偏移量为 0 
// 4   # i1 的偏移量为 4， c  -> i1 中间填充了 3个字节，才满足 4 字节的内存对齐要求
// 8   # i2 的偏移量为 8,  i1 -> i2 无填充
// 16  # l 的偏移量为 16， i2 -> l  中间填充了4个字节，才满足8字节的内存对齐要求
```

##### 如何手动设定内存对齐大小

首先，编译器基本上会默认到16字节的对齐需求，而在`C++17`中可以用如下宏来检查编译器默认的对齐大小，这个宏的大小不可更改（至少目前是这样）。在`Clang`中提供了一个编译选项`fnew-alignment`来更改内存对齐大小，但是`GCC`目前并不支持。

所以我们只能`case-by-case`的做。以下分为两种情况讨论：栈和堆。

###### static

```cpp
#if defined(__GNUC__)
#define CACHE_ALIGNED __attribute__((aligned(64))) // clang and GCC
#elif defined(_MSC_VER)
#define CACHE_ALIGNED __declspec(align(64))        // MSVC
#endif

alignas(64) int a[4];

int CACHE_ALIGNED a[4];

int a[4] CACHE_ALIGNED;

typedef CACHE_ALIGNED int aligned_t;
aligned_t option_4[4];
```

> `alignas`的实现就是利用的`__attribute_((aligned(64)))`指令。

对于结构体，我们也可以用相同的办法：

```cpp
struct AlignedField {
  alignas(64) int field;
};
AlignedField solution_1[4];
struct alignas(64) AlignedStruct {
  int field;
};
AlignedStruct solution_2[4];
```

<img src="https://miro.medium.com/v2/resize:fit:1400/1*n1YsJnG_Ns3031i7tmay_w.png" alt="img" style="zoom:50%;" />

###### dynamic

此时有两种选择：

1. `operator new + operator delete`
2. `placement new`

日常使用的`new operator`可以被分解为两个步骤：首先使用`operator new`分配内存，之后利用`placement new`调用对象的构造函数，并将对象指针指向先前分配的内存块中。

如果这里我们使用`operator new`：

```cpp
void* operator new  ( std::size_t count, std::align_val_t al);
void* operator new[]  ( std::size_t count, std::align_val_t al);

void* operator delete  ( void* ptr, std::size_t count, std::align_val_t al);
void* operator delete[]  ( void* ptr, std::size_t count, std::align_val_t al);

// 在这份代码中
float* resultSIMD = reinterpret_cast<float*>(::operator new[](N * sizeof(float), std::align_val_t{ 32 }));
```

如果使用`placement new`，则在释放内存时可能需要考虑手动调用析构函数，这里直接调用`delete[]`会报错。

> `delete[]`会先调用析构函数，再调用`operator delete`释放内存块，这部分可见CS 106L的笔记。

```cpp
auto pAlignedType= new(std::align_val_t{ 32 }) MyType;
pAlignedType->~MyType();
::operator delete(pAlignedType, std::align_val_t{32});

// 在这份代码中：
float* resultSIMD = new(std::align_val_t{ 32 }) float[N];
::operator delete[](resultSIMD, N * sizeof(float), std::align_val_t{ 32 });
```

##### std::align

C++11还提供了`align`函数用于在已经分配的缓存区中得到内存对齐的指针：

```cpp
// alignment	-	the desired alignment
// size	-	the size of the storage to be aligned
// ptr	-	pointer to contiguous storage (a buffer) of at least space bytes
// space	-	the size of the buffer in which to operate
    
template <std::size_t N>
struct MyAllocator
{
    char data[N];
    void* p;
    std::size_t sz;
    MyAllocator() : p(data), sz(N) {}
    template <typename T>
    T* aligned_alloc(std::size_t a = alignof(T))
    {
        if (std::align(a, sizeof(T), p, sz))
        {
            T* result = reinterpret_cast<T*>(p);
            p = (char*)p + sizeof(T);
            sz -= sizeof(T);
            return result;
        }
        return nullptr;
    }
};
```



