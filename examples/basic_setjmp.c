#include <stdio.h>
#include <setjmp.h>

jmp_buf jump_buffer;

void function_with_error() {
    printf("进入可能出错的函数\n");
    
    // 模拟某种错误条件
    int error_occurred = 1;
    
    if (error_occurred) {
        printf("检测到错误，准备跳转\n");
        longjmp(jump_buffer, 1);  // 跳转回 setjmp 调用点，返回值为 1
    }
    
    printf("函数正常执行完成\n");  // 这行不会执行
}

int main() {
    printf("程序开始执行\n");
    
    // 设置跳转点
    int result = setjmp(jump_buffer);
    
    if (result == 0) {
        // 第一次调用 setjmp，返回 0
        printf("首次设置跳转点，继续正常执行\n");
        function_with_error();
        printf("函数调用后继续执行\n");  // 这行不会执行
    } else {
        // 通过 longjmp 返回，result 为 longjmp 的第二个参数
        printf("通过 longjmp 返回，错误代码: %d\n", result);
    }
    
    printf("程序结束\n");
    return 0;
}