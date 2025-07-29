#include <stdio.h>
#include <setjmp.h>

jmp_buf coro_buf, main_buf;
int coro_step = 0;

void simple_coroutine() {
    switch (coro_step) {
        case 0:
            printf("协程: 第一次执行\n");
            coro_step = 1;
            if (setjmp(coro_buf) == 0) {
                longjmp(main_buf, 1);  // 返回主程序
            }
            // fallthrough
        case 1:
            printf("协程: 第二次执行\n");
            coro_step = 2;
            if (setjmp(coro_buf) == 0) {
                longjmp(main_buf, 2);  // 返回主程序
            }
            // fallthrough
        case 2:
            printf("协程: 第三次执行\n");
            coro_step = 3;
            longjmp(main_buf, 3);  // 最后一次返回
    }
}

int main() {
    printf("简单协程示例\n");
    printf("============\n");
    
    int result;
    
    // 第一次调用协程
    printf("主程序: 调用协程第一次\n");
    result = setjmp(main_buf);
    if (result == 0) {
        simple_coroutine();
    } else {
        printf("主程序: 协程返回，值=%d\n", result);
    }
    
    // 第二次调用协程
    if (coro_step < 3) {
        printf("主程序: 调用协程第二次\n");
        result = setjmp(main_buf);
        if (result == 0) {
            longjmp(coro_buf, 1);  // 恢复协程
        } else {
            printf("主程序: 协程返回，值=%d\n", result);
        }
    }
    
    // 第三次调用协程
    if (coro_step < 3) {
        printf("主程序: 调用协程第三次\n");
        result = setjmp(main_buf);
        if (result == 0) {
            longjmp(coro_buf, 1);  // 恢复协程
        } else {
            printf("主程序: 协程返回，值=%d\n", result);
        }
    }
    
    printf("主程序: 协程执行完成\n");
    return 0;
}