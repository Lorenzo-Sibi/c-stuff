CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I

OBJ_DIR = obj
TEST_DIR = tests

SRCS = jsont.c jsonp.c
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

TEST_SRCS = $(TEST_DIR)/test_main.c
TEST_OBJS = $(OBJ_DIR)/test_main.o

TARGET = json_lib

all: $(TARGET)

tokenizer: jsont.c
	gcc -Wall -Wextra -Og -g jsont.c -o jsont

parser: jsonp.c jsont.h
	gcc -Wall -Wextra -Og -g jsonp.c -o jsonp

clean:
	rm -f jsont jsonp *.o