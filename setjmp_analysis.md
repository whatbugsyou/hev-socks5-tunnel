# setjmp/longjmp 解析

## 概述

`setjmp` 和 `longjmp` 是 C 标准库中的两个函数，用于实现非局部跳转（non-local jumps）。它们提供了一种在 C 程序中实现类似于其他语言中异常处理机制的方法。

## 基本概念

### 函数原型

```c
#include <setjmp.h>

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
```

### 核心数据结构

```c
typedef struct {
    // 平台相关的寄存器保存结构
    // 通常包含：
    // - 程序计数器 (PC/IP)
    // - 栈指针 (SP)
    // - 帧指针 (FP/BP)
    // - 其他通用寄存器
    // - 状态寄存器
} jmp_buf[1];
```

## 工作原理

### setjmp 的工作机制

1. **保存执行上下文**：
   - 保存当前的寄存器状态
   - 保存栈指针位置
   - 保存程序计数器
   - 保存其他必要的处理器状态

2. **返回值机制**：
   - 直接调用时返回 0
   - 通过 longjmp 返回时返回 longjmp 的第二个参数

### longjmp 的工作机制

1. **恢复执行上下文**：
   - 从 jmp_buf 中恢复寄存器状态
   - 恢复栈指针（实际上是回退栈）
   - 跳转到 setjmp 的调用点

2. **传递返回值**：
   - 使 setjmp 返回指定的值（非零）

## 典型应用场景

### 1. 错误处理机制

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf error_buf;

void process_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        longjmp(error_buf, 1); // 跳转到错误处理
    }
    
    // 文件处理逻辑
    char buffer[256];
    if (!fgets(buffer, sizeof(buffer), file)) {
        fclose(file);
        longjmp(error_buf, 2); // 读取错误
    }
    
    fclose(file);
    printf("文件处理成功: %s\n", buffer);
}

int main() {
    int error_code = setjmp(error_buf);
    
    if (error_code == 0) {
        // 正常执行路径
        process_file("test.txt");
        printf("程序正常执行完毕\n");
    } else {
        // 错误处理路径
        printf("发生错误，错误代码: %d\n", error_code);
    }
    
    return 0;
}
```

### 2. 协程实现

```c
#include <stdio.h>
#include <setjmp.h>

#define MAX_COROUTINES 10

typedef struct {
    jmp_buf context;
    int active;
    int id;
} coroutine_t;

coroutine_t coroutines[MAX_COROUTINES];
int current_coroutine = -1;
int main_context_saved = 0;
jmp_buf main_context;

void yield_to(int target_id) {
    if (current_coroutine >= 0) {
        // 保存当前协程状态
        if (setjmp(coroutines[current_coroutine].context) == 0) {
            current_coroutine = target_id;
            if (target_id == -1) {
                // 返回主函数
                longjmp(main_context, 1);
            } else {
                // 跳转到目标协程
                longjmp(coroutines[target_id].context, 1);
            }
        }
    }
}

void coroutine_func_1() {
    for (int i = 0; i < 3; i++) {
        printf("协程1执行: %d\n", i);
        yield_to(1); // 切换到协程2
    }
    printf("协程1完成\n");
    yield_to(-1); // 返回主函数
}

void coroutine_func_2() {
    for (int i = 0; i < 3; i++) {
        printf("协程2执行: %d\n", i);
        yield_to(0); // 切换到协程1
    }
    printf("协程2完成\n");
    yield_to(-1); // 返回主函数
}

int main() {
    // 初始化协程
    coroutines[0].active = 1;
    coroutines[0].id = 0;
    coroutines[1].active = 1;
    coroutines[1].id = 1;
    
    // 保存主函数上下文
    if (setjmp(main_context) == 0) {
        current_coroutine = 0;
        // 启动第一个协程
        if (setjmp(coroutines[0].context) == 0) {
            coroutine_func_1();
        }
    }
    
    // 检查是否需要启动第二个协程
    if (current_coroutine == 1) {
        if (setjmp(coroutines[1].context) == 0) {
            coroutine_func_2();
        }
    }
    
    printf("主函数结束\n");
    return 0;
}
```

## 实现细节分析

### 平台相关的实现

#### x86-64 平台的 jmp_buf 结构

```c
// 典型的 x86-64 jmp_buf 实现
typedef struct {
    long long rbx;     // 通用寄存器
    long long rbp;     // 基址指针
    long long r12;     // 通用寄存器
    long long r13;     // 通用寄存器  
    long long r14;     // 通用寄存器
    long long r15;     // 通用寄存器
    long long rsp;     // 栈指针
    long long rip;     // 指令指针
} __jmp_buf[1];
```

#### ARM 平台的实现考虑

```c
// ARM 平台需要保存的寄存器
typedef struct {
    long r4, r5, r6, r7, r8, r9, r10, r11; // 通用寄存器
    long sp;                                // 栈指针
    long lr;                                // 链接寄存器
    // 可能还需要保存浮点寄存器状态
} arm_jmp_buf[1];
```

### setjmp 的汇编实现示例 (x86-64)

```assembly
; setjmp 的简化汇编实现
setjmp:
    movq %rbx, 0(%rdi)    ; 保存 rbx
    movq %rbp, 8(%rdi)    ; 保存 rbp
    movq %r12, 16(%rdi)   ; 保存 r12
    movq %r13, 24(%rdi)   ; 保存 r13
    movq %r14, 32(%rdi)   ; 保存 r14
    movq %r15, 40(%rdi)   ; 保存 r15
    movq %rsp, 48(%rdi)   ; 保存栈指针
    movq (%rsp), %rax     ; 获取返回地址
    movq %rax, 56(%rdi)   ; 保存返回地址
    xorq %rax, %rax       ; 返回 0
    ret
```

### longjmp 的汇编实现示例 (x86-64)

```assembly
; longjmp 的简化汇编实现
longjmp:
    movq %rsi, %rax       ; 设置返回值
    movq 0(%rdi), %rbx    ; 恢复 rbx
    movq 8(%rdi), %rbp    ; 恢复 rbp
    movq 16(%rdi), %r12   ; 恢复 r12
    movq 24(%rdi), %r13   ; 恢复 r13
    movq 32(%rdi), %r14   ; 恢复 r14
    movq 40(%rdi), %r15   ; 恢复 r15
    movq 48(%rdi), %rsp   ; 恢复栈指针
    movq 56(%rdi), %rdx   ; 获取返回地址
    jmpq *%rdx            ; 跳转到保存的地址
```

## 使用注意事项

### 1. 栈展开问题

```c
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

jmp_buf buf;

void dangerous_function() {
    char *ptr = malloc(1024);  // 分配内存
    
    // 如果在这里调用 longjmp，ptr 不会被释放
    // 造成内存泄漏
    
    longjmp(buf, 1);
    
    free(ptr);  // 这行代码永远不会执行
}

int main() {
    if (setjmp(buf) == 0) {
        dangerous_function();
    } else {
        printf("从 longjmp 返回\n");
        // 内存泄漏已经发生
    }
    return 0;
}
```

### 2. 局部变量的状态

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf buf;

void test_local_vars() {
    int local_var = 10;
    volatile int volatile_var = 20;  // volatile 保证不被优化
    
    printf("调用 setjmp 前: local_var=%d, volatile_var=%d\n", 
           local_var, volatile_var);
    
    if (setjmp(buf) == 0) {
        local_var = 100;
        volatile_var = 200;
        printf("修改后: local_var=%d, volatile_var=%d\n", 
               local_var, volatile_var);
        longjmp(buf, 1);
    } else {
        printf("longjmp 返回后: local_var=%d, volatile_var=%d\n", 
               local_var, volatile_var);
        // local_var 的值是未定义的
        // volatile_var 的值是 200
    }
}

int main() {
    test_local_vars();
    return 0;
}
```

### 3. 信号处理中的使用

```c
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

sigjmp_buf sig_buf;

void signal_handler(int sig) {
    printf("收到信号 %d\n", sig);
    siglongjmp(sig_buf, 1);  // 使用 siglongjmp 恢复信号掩码
}

int main() {
    signal(SIGINT, signal_handler);
    
    if (sigsetjmp(sig_buf, 1) == 0) {  // 保存信号掩码
        printf("程序运行中，按 Ctrl+C 测试信号处理\n");
        while (1) {
            sleep(1);
            printf("运行中...\n");
        }
    } else {
        printf("从信号处理程序返回\n");
    }
    
    return 0;
}
```

## 性能考虑

### 1. setjmp 的开销

```c
#include <stdio.h>
#include <setjmp.h>
#include <time.h>

#define ITERATIONS 10000000

int main() {
    jmp_buf buf;
    clock_t start, end;
    
    // 测试 setjmp 的开销
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        setjmp(buf);
    }
    end = clock();
    
    printf("setjmp 调用 %d 次耗时: %f 秒\n", 
           ITERATIONS, ((double)(end - start)) / CLOCKS_PER_SEC);
    
    return 0;
}
```

### 2. 与函数调用的比较

```c
#include <stdio.h>
#include <setjmp.h>
#include <time.h>

#define ITERATIONS 10000000

jmp_buf buf;

void empty_function() {
    // 空函数用于比较
}

void longjmp_test() {
    longjmp(buf, 1);
}

int main() {
    clock_t start, end;
    
    // 测试普通函数调用
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        empty_function();
    }
    end = clock();
    printf("函数调用耗时: %f 秒\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    
    // 测试 setjmp/longjmp
    start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        if (setjmp(buf) == 0) {
            longjmp_test();
        }
    }
    end = clock();
    printf("setjmp/longjmp 耗时: %f 秒\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    
    return 0;
}
```

## 现代替代方案

### 1. C++ 异常处理

```cpp
#include <iostream>
#include <exception>

class FileError : public std::exception {
private:
    std::string message;
public:
    FileError(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

void process_file(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw FileError("无法打开文件");
    }
    
    std::string line;
    if (!std::getline(file, line)) {
        throw FileError("文件读取失败");
    }
    
    std::cout << "文件内容: " << line << std::endl;
}

int main() {
    try {
        process_file("test.txt");
        std::cout << "程序正常执行完毕" << std::endl;
    } catch (const FileError& e) {
        std::cout << "文件错误: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### 2. 现代协程库

```c
// 使用 libco 或其他现代协程库
#include "co_routine.h"

void* coroutine_func(void* arg) {
    for (int i = 0; i < 5; i++) {
        printf("协程执行: %d\n", i);
        co_yield_ct();  // 让出执行权
    }
    return NULL;
}

int main() {
    stCoRoutine_t* co;
    co_create(&co, NULL, coroutine_func, NULL);
    
    for (int i = 0; i < 10; i++) {
        co_resume(co);
        printf("主函数执行: %d\n", i);
    }
    
    co_release(co);
    return 0;
}
```

## 调试技巧

### 1. 跟踪 setjmp/longjmp 调用

```c
#include <stdio.h>
#include <setjmp.h>

#define DEBUG_SETJMP 1

#if DEBUG_SETJMP
#define DEBUG_SETJMP_CALL(buf) \
    do { \
        printf("DEBUG: setjmp called at %s:%d\n", __FILE__, __LINE__); \
        int result = setjmp(buf); \
        printf("DEBUG: setjmp returned %d\n", result); \
        if (result != 0) { \
            printf("DEBUG: returned from longjmp with value %d\n", result); \
        } \
        return result; \
    } while(0)

#define DEBUG_LONGJMP_CALL(buf, val) \
    do { \
        printf("DEBUG: longjmp called at %s:%d with value %d\n", \
               __FILE__, __LINE__, val); \
        longjmp(buf, val); \
    } while(0)
#else
#define DEBUG_SETJMP_CALL(buf) setjmp(buf)
#define DEBUG_LONGJMP_CALL(buf, val) longjmp(buf, val)
#endif

jmp_buf error_buf;

void test_function() {
    printf("进入 test_function\n");
    DEBUG_LONGJMP_CALL(error_buf, 42);
    printf("这行不会被执行\n");
}

int main() {
    int result = DEBUG_SETJMP_CALL(error_buf);
    
    if (result == 0) {
        printf("正常执行路径\n");
        test_function();
    } else {
        printf("异常处理路径，返回值: %d\n", result);
    }
    
    return 0;
}
```

## 总结

`setjmp/longjmp` 是 C 语言中强大但危险的控制流机制：

**优点：**
- 提供非局部跳转能力
- 可以实现简单的异常处理
- 可以用于实现协程和状态机
- 性能相对较好

**缺点：**
- 容易导致内存泄漏和资源泄漏
- 局部变量状态不确定
- 代码难以理解和维护
- 破坏了结构化编程原则

**建议：**
- 现代 C++ 程序优先使用异常处理
- 系统编程中谨慎使用，确保正确的资源管理
- 考虑使用现代协程库替代自实现
- 充分测试和文档化使用场景

`setjmp/longjmp` 虽然功能强大，但在现代编程中应该谨慎使用，更多地作为理解底层机制和特殊场景的工具。