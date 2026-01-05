
CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -std=c11 -MMD -MP

SRC_DIR := src
BIN_DIR := bin
INC_DIR := include

COMPILER = bfCompiler
TESTER = bfTester

COMMON_SRCS = interp.c parser.c utils.c test.c
COMPILER_SRCS = main.c $(COMMON_SRCS)
TESTER_SRCS = runTests.c $(COMMON_SRCS)

COMPILER_OBJS = $(COMPILER_SRCS:%.c=$(BIN_DIR)/%.o)
TESTER_OBJS = $(TESTER_SRCS:%.c=$(BIN_DIR)/%.o)

DEPS = interp.h parser.h utils.h test.h

all: $(COMPILER) $(TESTER)

$(COMPILER): $(COMPILER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TESTER): $(TESTER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR) #$(DEPS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(COMPILER) $(TESTER)
