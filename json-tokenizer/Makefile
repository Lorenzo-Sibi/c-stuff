CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

OBJ_DIR  = obj
TEST_DIR = tests
JSON_TEST_DIR = $(TEST_DIR)/json_files_test

SRCS = jsonp.c
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

TEST_SRCS = $(TEST_DIR)/test_main.c
TEST_OBJS = $(OBJ_DIR)/test_main.o

all: parser

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@
	
tokenizer: jsont.h
	$(CC) -Wall -Wextra -Og -g jsont.h -o jsont

parser: jsonp.c jsont.h
	$(CC) -Wall -Wextra -Og -g -DJSONP_DEMO jsonp.c -o jsonp

# Test suite: builds the library objects + tests/test_main.c
test: $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o test_runner $(TEST_OBJS) $(OBJS)
	./test_runner

$(OBJ_DIR)/test_main.o: $(TEST_DIR)/test_main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

clean:
	rm -f jsont jsonp test_runner
	rm -rf $(OBJ_DIR)

.PHONY: all clean test tokenizer parser