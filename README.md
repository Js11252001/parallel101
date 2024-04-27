## Homework1
问：为什么stbi库要这样设计？

答：要在根目录下无痛使用使用stbi，即无需对代码进行任何更改，只需在stbi目录下的CmakeList中定义**stbi**的头文件搜索路径，即`target_include_directories(stbi PUBLIC .)`
，其中PUBLIC决定了该属性在link时会被传播。这样设计有以下几点好处：

- 对于引用他的可执行文件，CMake会自动添加这个路径，无需再每个引用他的Cmake文件中再指定一遍。
- 通过这样引用之后，在其他地方可以用 <> 来引用这个头文件了，因为这样指定的路径会被视为与系统路径等价。
