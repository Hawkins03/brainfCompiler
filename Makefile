
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -MMD -MP

COMPILER = bfCompiler
TESTER = bfTester

COMMON_SRCS = interp.c parser.c utils.c

COMPILER_SRCS = main.c $(COMMON_SRCS)
TESTER_SRCS = runtests.c $(COMMON_SRCS) test.c
DEPS = interp.h parser.h utils.h test.h

COMPILER_OBJS = $(COMPILER_SRCS:.c=.o)
TESTER_OBJS = $(TESTER_SRCS:.c=.o)

all: $(COMPILER) $(TESTER)

$(COMPILER): $(COMPILER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TESTER): $(TESTER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.d *.bak $(COMPILER_OBJS) $(TESTER_OBJS) $(COMPILER) $(TESTER)
