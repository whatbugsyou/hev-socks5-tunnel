CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET_DIR = bin

# 目标文件列表
TARGETS = basic_setjmp error_handling simple_coro

# 默认目标
all: $(TARGET_DIR) $(addprefix $(TARGET_DIR)/, $(TARGETS))

# 创建目标目录
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

# 编译规则
$(TARGET_DIR)/%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# 清理目标
clean:
	rm -rf $(TARGET_DIR)

# 运行所有示例
run: all
	@echo "========================================="
	@echo "运行基本 setjmp 示例:"
	@echo "========================================="
	./$(TARGET_DIR)/basic_setjmp
	@echo ""
	@echo "========================================="
	@echo "运行错误处理示例:"
	@echo "========================================="
	./$(TARGET_DIR)/error_handling
	@echo ""
	@echo "========================================="
	@echo "运行简单协程示例:"
	@echo "========================================="
	./$(TARGET_DIR)/simple_coro

# 单独运行示例
run-basic: $(TARGET_DIR)/basic_setjmp
	./$(TARGET_DIR)/basic_setjmp

run-error: $(TARGET_DIR)/error_handling
	./$(TARGET_DIR)/error_handling

run-coroutine: $(TARGET_DIR)/simple_coro
	./$(TARGET_DIR)/simple_coro

.PHONY: all clean run run-basic run-error run-coroutine