## Homework1
问：为什么stbi库要这样设计？

答：要在根目录下无痛使用使用stbi，即无需对代码进行任何更改，只需在stbi目录下的CmakeList中定义**stbi**的头文件搜索路径，即`target_include_directories(stbi PUBLIC .)`
，其中PUBLIC决定了该属性在link时会被传播。这样设计有以下几点好处：

- 对于引用他的可执行文件，CMake会自动添加这个路径，无需再每个引用他的Cmake文件中再指定一遍。
- 通过这样引用之后，在其他地方可以用 <> 来引用这个头文件了，因为这样指定的路径会被视为与系统路径等价。


## Homework2
- 避免函数参数不必要的拷贝 5 分
  答：使用const &
- 修复智能指针造成的问题 10 分
  答：避免同时使用shared_ptr相互引用的情况, 可使用weak_ptr 或者unique_ptr+ raw的方案解决。
- 改用 `unique_ptr<Node>` 10 分
  答：同上。在实现时注意如果在move之后还想访问原对象，那么需要提交获取原指针。
- 实现拷贝构造函数为深拷贝 15 分
  答：浅拷贝对于成员变量包含指针的类，会造成资源会被释放两次！更危险的则是 v1 被解构而 v2 仍在被使用的情况。因此需要深拷贝。
- 说明为什么可以删除拷贝赋值函数 5 分
  答：会调用拷贝构造函数生成临时对象，之后通过移动赋值函数获取对象。
- 改进 `Node` 的构造函数 5 分
  答：建议使用初始化列表。

## Homework3
-作业核心问题：通过variant和visit实现加法的函数重载

答：对于两个variant之间的加法可直接通过visit批量匹配实现，对于其中有一个不是variant的情况，可通过函数重载来实现加法。对于variant和另一个不匹配的情况，编译时会直接报错，这个由variant保证。std::visit、std::variant 的这种模式称为静态多态，因此在编译时，std::visit 会自动罗列出加法的所有的排列组合。

## Homework4
优化代码技巧：将AOS转为SOA, 使用msvc下的编译优化，对结构体进行2的整数次幂对齐，使用unroll，将某些变量移动到外循环，不变量在循坏外做好预计算。

## Homework5
- 把 `login`, `register` 等函数变成多线程安全的，能利用 `shared_mutex` 区分读和写
  
  答：使用shared_mutex和unique_lock控制写函数, 用shared_mutex和shared_lock控制读函数，并且符合RAII。
- 把 `login` 的登录计时器改成基于 chrono 的
  
  答：使用c++chrono库的相关函数
- 让 ThreadPool::create 创建的线程保持后台运行不要退出,等待 tpool 中所有线程都结束后再退出
  
  答：使用一个数组，在ThreadPool析构时join每个thread

## Homework6
并行for循环，对于简单的映射，a[i] = val使用parallel_for。

对于求和的情况，使用parallel_reduce。

对求最值的并行化，可以使用reduce来归并。

对于并行筛选的情况，可使用parallel_for配合局部数组。

对于要存储中间值的数组，可以使用parallel_scan。
