// 这是第07课的回家作业，主题是访存优化
// 录播见：https://www.bilibili.com/video/BV1gu41117bW
// 作业的回答推荐写在 ANSWER.md 方便老师批阅，也可在 PR 描述中
// 请晒出程序被你优化前后的运行结果（ticktock 的用时统计）
// 可以比较采用了不同的优化手段后，加速了多少倍，做成表格
// 如能同时贴出 CPU 核心数量，缓存大小等就最好了（lscpu 命令）
// 作业中有很多个问句，请通过注释回答问题，并改进其代码，以使其更快
// 并行可以用 OpenMP 也可以用 TBB

#include <iostream>
//#include <x86intrin.h>  // _mm 系列指令都来自这个头文件
#include <xmmintrin.h>  // 如果上面那个不行，试试这个
#include "ndarray.h"
#include "wangsrng.h"
#include "ticktock.h"

// Matrix 是 YX 序的二维浮点数组：mat(x, y) = mat.data()[y * mat.shape(0) + x]
using Matrix = ndarray<2, float>;
// 注意：默认对齐到 64 字节，如需 4096 字节，请用 ndarray<2, float, AlignedAllocator<4096, float>>

static void matrix_randomize(Matrix &out) {
    TICK(matrix_randomize);
    size_t nx = out.shape(0);
    size_t ny = out.shape(1);

    // 这个循环为什么不够高效？如何优化？ 10 分 
    //跳跃访存，使缓存无法命中从而增加消耗。
    //Matrix 是 YX 序，所以外层循环应该先遍历y，利用顺序访问的;
    //然后使用sse指令集并行处理：绕过缓存直接写入避免写入粒度太小造成不必要读取
#pragma omp parallel for collapse(2)
    for (int y = 0; y < nx; y++) {
        for (int x = 0; x < ny; x += 4) {
            __m128 val = _mm_set_ps(
                wangsrng(x + 0, y).next_float(),
                wangsrng(x + 1, y).next_float(),
                wangsrng(x + 2, y).next_float(),
                wangsrng(x + 3, y).next_float()
            );
            _mm_stream_ps(&out(x, y), val);
        }
    }
    TOCK(matrix_randomize);
}

static void matrix_transpose(Matrix &out, Matrix const &in) {
    TICK(matrix_transpose);
    size_t nx = in.shape(0);
    size_t ny = in.shape(1);
    out.reshape(ny, nx);

    // 这个循环为什么不够高效？如何优化？ 15 分
    //因为不管是xy还是yx序，都会有一个数组违背空间局域性
    //使用循坏分块法
#pragma omp parallel for collapse(2)
    for (int x = 0; x < nx; x+=32) {
        for (int y = 0; y < ny; y+=32) {
            for (int x_ = x; x_ < x + 32; x_++) {
                for (int y_ = y; y_ < y + 32; y_++) {
                    out(y_, x_) = in(x_, y_);
                }
            }
        }
    }
    TOCK(matrix_transpose);
}

static void matrix_multiply(Matrix &out, Matrix const &lhs, Matrix const &rhs) {
    TICK(matrix_multiply);
    size_t nx = lhs.shape(0);
    size_t nt = lhs.shape(1);
    size_t ny = rhs.shape(1);
    if (rhs.shape(0) != nt) {
        std::cerr << "matrix_multiply: shape mismatch" << std::endl;
        throw;
    }
    out.reshape(nx, ny);

    // 这个循环为什么不够高效？如何优化？ 15 分
    //对于lhs来说每次访问跳读，对out(x,y)无法cpu并行，性能不好。
    //使用寄存器分块这样就消除不连续的访问了，从而内部的循环可以顺利矢量化，
    //且多个循环体之间没有依赖关系，CPU得以启动指令级并行，缓存预取也能正常工作
#pragma omp parallel for collapse(2)
    for (int y = 0; y < ny; y++) {
        for (int x_base = 0; x_base < nx; x_base+= 32) {
            //out(x, y) = 0;  // 有没有必要手动初始化？ 5 分 不需要
            for (int t = 0; t < nt; t++) {
                for (int x = x_base; x < x_base +32; x++)
                    out(x, y) += lhs(x, t) * rhs(t, y);
            }
        }
    }
    TOCK(matrix_multiply);
}

// 求出 R^T A R
static void matrix_RtAR(Matrix &RtAR, Matrix const &R, Matrix const &A) {
    TICK(matrix_RtAR);
    // 这两个是临时变量，有什么可以优化的？ 5 分//手动池化 声明为static并避免重复分配
    static thread_local Matrix Rt, RtA;
    matrix_transpose(Rt, R);
    matrix_multiply(RtA, Rt, A);
    matrix_multiply(RtAR, RtA, R);
    TOCK(matrix_RtAR);
}

static float matrix_trace(Matrix const &in) {
    TICK(matrix_trace);
    float res = 0;
    size_t nt = std::min(in.shape(0), in.shape(1));
#pragma omp parallel for reduction(+:res)
    for (int t = 0; t < nt; t++) {
        res += in(t, t);
    }
    TOCK(matrix_trace);
    return res;
}

static void test_func(size_t n) {
    TICK(test_func);
    Matrix R(n, n);
    matrix_randomize(R);
    Matrix A(n, n);
    matrix_randomize(A);

    Matrix RtAR;
    matrix_RtAR(RtAR, R, A);

    std::cout << matrix_trace(RtAR) << std::endl;
    TOCK(test_func);
}

int main() {
    wangsrng rng;
    TICK(overall);
    for (int t = 0; t < 4; t++) {
        size_t n = 32 * (rng.next_uint64() % 16 + 24);
        std::cout << "t=" << t << ": n=" << n << std::endl;
        test_func(n);
    }
    TOCK(overall);
    return 0;
}
