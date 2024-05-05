#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include "ticktock.h"
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_scan.h>
#include "pod.h"
//cpu: 8core 16thread

// TODO: 并行化所有这些 for 循环

template <class T, class Func>
std::vector<T> fill(std::vector<T> &arr, Func const &func) {
    TICK(fill);
    //for (size_t i = 0; i < arr.size(); i++) {
    //    arr[i] = func(i);
    //}
    tbb::parallel_for((size_t)0, (size_t)arr.size(), [&](size_t i) {
        arr[i] = func(i);
    });
    TOCK(fill);
    return arr;
}

template <class T>
void saxpy(T a, std::vector<T> &x, std::vector<T> const &y) {
    TICK(saxpy);
    //for (size_t i = 0; i < x.size(); i++) {
    //   x[i] = a * x[i] + y[i];
    //}
    tbb::parallel_for((size_t)0, (size_t)x.size(), [&](size_t i) {
        x[i] = a * x[i] + y[i];
        });
    TOCK(saxpy);
}

template <class T>
T sqrtdot(std::vector<T> const &x, std::vector<T> const &y) {
    TICK(sqrtdot);
    //T ret = 0;
    //for (size_t i = 0; i < std::min(x.size(), y.size()); i++) {
    //    ret += x[i] * y[i];
    //}
    //ret = std::sqrt(ret);
    T ret = tbb::parallel_reduce(tbb::blocked_range<size_t>(0, std::min(x.size(), y.size())), T{},
        [&](tbb::blocked_range<size_t> r, T local_res) {
            for (size_t i = r.begin(); i < r.end(); i++) {
                local_res += x[i] * y[i];
            }
            return local_res;
        }, [](T a, T b) {
            return a + b;
        });
    ret = std::sqrt(ret);
    TOCK(sqrtdot);
    return ret;
}

template <class T>
T minvalue(std::vector<T> const &x) {
    TICK(minvalue);
    //T ret = x[0];
    //for (size_t i = 1; i < x.size(); i++) {
    //    if (x[i] < ret)
    //        ret = x[i];
    //}
    T ret = tbb::parallel_reduce(tbb::blocked_range<size_t>((size_t)1, x.size()), T{x[0]},
        [&](tbb::blocked_range<size_t> r, T local_res) {
            for (size_t i = r.begin(); i < r.end(); i++) {
                local_res = local_res < x[i] ? local_res : x[i];
            }
            return local_res;
        }, [](T a, T b) {
            return a < b ? a : b;
        });
    TOCK(minvalue);
    return ret;
}

template <class T>
std::vector<pod<T>> magicfilter(std::vector<T> const &x, std::vector<T> const &y) {
    TICK(magicfilter);
    std::vector<pod<T>> res;
    std::atomic<size_t> a_size = 0;
    //for (size_t i = 0; i < std::min(x.size(), y.size()); i++) {
    //    if (x[i] > y[i]) {
    //        res.push_back(x[i]);
    //    } else if (y[i] > x[i] && y[i] > 0.5f) {
    //        res.push_back(y[i]);
    //        res.push_back(x[i] * y[i]);
    //    }
    //}
    auto n = std::min(x.size(), y.size());
    res.resize(n);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
        [&](tbb::blocked_range<size_t> r) {
            std::vector<pod<T>> local_a(r.size() + 1);
            size_t lasize = 0;
            for (size_t i = r.begin(); i < r.end(); i++) {
                float val = x[i];
                float val2 = y[i];
                if (x[i] > y[i]) {
                    local_a[lasize++] = val;
                } else if (y[i] > x[i] && y[i] > 0.5f) {
                    local_a[lasize++] = val2;
                    local_a[lasize++] = val * val2;
                }               
            }
            size_t base = a_size.fetch_add(lasize);
            for (size_t i = 0; i < lasize; i++)
            {
                res[base + i] = local_a[i];
            }
        });
    res.resize(a_size);
    TOCK(magicfilter);
    return res;
}

template <class T>
T scanner(std::vector<T> &x) {
    TICK(scanner);
    //T ret = 0;
    //for (size_t i = 0; i < x.size(); i++) {
    //    ret += x[i];
    //    x[i] = ret;
    //}
    T ret = tbb::parallel_scan(tbb::blocked_range<size_t>(0, x.size()), T{},
        [&](tbb::blocked_range<size_t> r, T local_res, auto is_final) {
            for (size_t i = r.begin(); i < r.end(); i++) {
                local_res += x[i];
                if (is_final)
                    x[i] = local_res;
            }
            return local_res;
        }, [](T a, T b) {
            return a + b;
        });
    TOCK(scanner);
    return ret;
}

int main() {
    size_t n = 1<<26;
    std::vector<float> x(n);
    std::vector<float> y(n);

    fill(x, [&] (size_t i) { return std::sin(i); });
    fill(y, [&] (size_t i) { return std::cos(i); });

    saxpy(0.5f, x, y);

    std::cout << sqrtdot(x, y) << std::endl;
    std::cout << minvalue(x) << std::endl;

    auto arr = magicfilter(x, y);
    std::cout << arr.size() << std::endl;

    scanner(x);
    std::cout << std::reduce(x.begin(), x.end()) << std::endl;

    return 0;
}
