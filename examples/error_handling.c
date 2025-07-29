#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

// 错误类型定义
#define ERROR_FILE_NOT_FOUND    1
#define ERROR_MEMORY_ALLOCATION 2
#define ERROR_INVALID_DATA      3

jmp_buf error_handler;

// 模拟文件操作
void process_file(const char* filename) {
    printf("尝试打开文件: %s\n", filename);
    
    // 模拟文件不存在的情况
    if (strcmp(filename, "nonexistent.txt") == 0) {
        longjmp(error_handler, ERROR_FILE_NOT_FOUND);
    }
    
    // 模拟内存分配
    char* buffer = malloc(1024);
    if (!buffer) {
        longjmp(error_handler, ERROR_MEMORY_ALLOCATION);
    }
    
    printf("文件处理中...\n");
    
    // 模拟数据验证错误
    if (strcmp(filename, "invalid.txt") == 0) {
        free(buffer);  // 确保释放内存
        longjmp(error_handler, ERROR_INVALID_DATA);
    }
    
    printf("文件处理成功完成\n");
    free(buffer);
}

void test_file_processing() {
    const char* files[] = {
        "good_file.txt",
        "nonexistent.txt",
        "invalid.txt",
        "another_good_file.txt"
    };
    
    int num_files = sizeof(files) / sizeof(files[0]);
    
    for (int i = 0; i < num_files; i++) {
        printf("\n--- 处理文件 %d: %s ---\n", i + 1, files[i]);
        
        int error_code = setjmp(error_handler);
        
        if (error_code == 0) {
            // 正常执行路径
            process_file(files[i]);
            printf("文件 %s 处理成功\n", files[i]);
        } else {
            // 错误处理路径
            switch (error_code) {
                case ERROR_FILE_NOT_FOUND:
                    printf("错误: 文件 %s 不存在\n", files[i]);
                    break;
                case ERROR_MEMORY_ALLOCATION:
                    printf("错误: 内存分配失败\n");
                    break;
                case ERROR_INVALID_DATA:
                    printf("错误: 文件 %s 包含无效数据\n", files[i]);
                    break;
                default:
                    printf("错误: 未知错误代码 %d\n", error_code);
                    break;
            }
        }
    }
}

int main() {
    printf("错误处理示例程序\n");
    printf("=================\n");
    
    test_file_processing();
    
    printf("\n程序结束\n");
    return 0;
}