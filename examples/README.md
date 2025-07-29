# setjmp/longjmp 示例程序

本目录包含了多个演示 `setjmp` 和 `longjmp` 功能的 C 程序示例。

## 编译和运行

### 编译所有示例
```bash
make all
```

### 运行所有示例
```bash
make run
```

### 单独编译和运行
```bash
# 编译单个示例
gcc -Wall -Wextra -std=c99 -g -o bin/basic_setjmp basic_setjmp.c

# 运行单个示例
./bin/basic_setjmp
```

## 示例说明

### 1. `basic_setjmp.c` - 基本用法示例

这个示例演示了 `setjmp/longjmp` 的基本用法：
- 使用 `setjmp` 设置跳转点
- 使用 `longjmp` 从深层函数调用中跳转回来
- 展示了 `setjmp` 的两种返回值（直接调用返回0，通过 `longjmp` 返回非零值）

**输出示例：**
```
程序开始执行
首次设置跳转点，继续正常执行
进入可能出错的函数
检测到错误，准备跳转
通过 longjmp 返回，错误代码: 1
程序结束
```

### 2. `error_handling.c` - 错误处理示例

这个示例展示了如何使用 `setjmp/longjmp` 实现类似异常处理的机制：
- 定义不同的错误类型
- 在错误发生时跳转到统一的错误处理代码
- 演示了资源管理的重要性

**特性：**
- 文件不存在错误
- 内存分配错误  
- 数据验证错误
- 统一的错误处理机制

### 3. `simple_coro.c` - 简单协程示例

这个示例演示了如何使用 `setjmp/longjmp` 实现简单的协程：
- 协程的暂停和恢复
- 主程序与协程之间的控制权切换
- 状态保存和恢复

**输出示例：**
```
简单协程示例
============
主程序: 调用协程第一次
协程: 第一次执行
主程序: 协程返回，值=1
主程序: 调用协程第二次
协程: 第二次执行
主程序: 协程返回，值=2
...
```

## 重要注意事项

### 1. 内存管理
使用 `longjmp` 时要特别注意内存泄漏问题：
```c
void dangerous_function() {
    char *ptr = malloc(1024);
    longjmp(buf, 1);  // ptr 不会被释放！
    free(ptr);        // 这行代码不会执行
}
```

### 2. 局部变量
`longjmp` 后局部变量的值可能不确定，除非声明为 `volatile`：
```c
int local_var = 10;           // 值可能不确定
volatile int safe_var = 20;   // 值会被保留
```

### 3. 栈展开
`longjmp` 不会调用 C++ 析构函数或清理栈帧上的资源，这可能导致资源泄漏。

### 4. 信号安全
在信号处理程序中，应该使用 `sigsetjmp/siglongjmp` 而不是 `setjmp/longjmp`。

## 现代替代方案

虽然 `setjmp/longjmp` 很强大，但在现代编程中有更好的替代方案：

- **C++ 异常处理**: 使用 try/catch 机制
- **错误码返回**: 传统的错误处理方式
- **现代协程库**: 如 libco、libcoro 等
- **状态机**: 更结构化的控制流管理

## 性能考虑

`setjmp` 的调用开销相对较大，因为需要保存大量的处理器状态。在性能敏感的代码中应谨慎使用。

## 编译选项

所有示例都使用以下编译选项：
- `-Wall -Wextra`: 启用所有警告
- `-std=c99`: 使用 C99 标准
- `-g`: 包含调试信息

## 学习建议

1. 先理解基本示例 (`basic_setjmp.c`)
2. 学习错误处理模式 (`error_handling.c`)
3. 了解协程概念 (`simple_coro.c`)
4. 阅读详细分析文档 (`../setjmp_analysis.md`)
5. 实验修改示例代码以加深理解

## 调试技巧

使用 GDB 调试这些程序时，可以观察栈帧的变化：
```bash
gdb ./bin/basic_setjmp
(gdb) break main
(gdb) run
(gdb) bt    # 查看调用栈
(gdb) next  # 单步执行
```