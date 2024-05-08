# 改进前

```
matrix_randomize: 0.0006219s
matrix_transpose: 0.0014136s
matrix_multiply: 0.621383s
matrix_RtAR: 1.2464s
```

# 改进后

```
matrix_randomize: 0.0002744s
matrix_transpose: 0.0004642s
matrix_multiply: 0.214093s
matrix_RtAR: 0.453685s
```


# 优化方法


> matrix_randomize

Matrix 是 YX 序，所以外层循环应该先遍历y，利用顺序访问的.使用sse指令集并行处理：绕过缓存直接写入避免写入粒度太小造成不必要读取

> matrix_transpose

因为不管是xy还是yx序，都会有一个数组违背空间局域性.使用循坏分块法,保证一个块在缓存中，因此加速。

> matrix_multiply

对于lhs来说每次访问跳读，对out(x,y)无法cpu并行，性能不好。使用寄存器分块这样就消除不连续的访问了，从而内部的循环可以顺利矢量化，且多个循环体之间没有依赖关系，CPU得以启动指令级并行，缓存预取也能正常工作

> matrix_RtAR

这两个是临时变量，有什么可以优化的？ 5 分//手动池化 声明为static 并避免重复分配，使用thread_local保证多线程调用不出错