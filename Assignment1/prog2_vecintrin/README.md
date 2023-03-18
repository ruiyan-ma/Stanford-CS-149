当不断增大`VECTOR_WIDTH`时，会发现`Vector Utilization`不断下降，这是因为`Vector Utilization`统计的是在`SIMD`并行时为`1`的掩码数量比例。当`VECTOR_WIDTH`增大时，就会导致有更多的`unutilized lanes`进入`while`循环中与同属于一个`simd`指令的`utilized lanes`并行，从而导致通路利用率的下降。