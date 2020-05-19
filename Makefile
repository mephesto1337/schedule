CC = gcc
CPPFLAGS = -D_GNU_SOURCE -Iinclude/
CFLAGS = -W -Wall -Wextra -Werror -std=c99 -fPIC

LD = gcc
LDFLAGS =
LIBS = 

TEST_CPPFLAGS = $(CPPFLAGS) -Isrc/
TEST_CFLAGS = $(CFLAGS)
TEST_LDFLAGS = $(LDFLAGS) -L$(PWD) -Wl,-rpath=$(PWD)
TEST_LIBS = $(LIBS) -lschedule -pthread

TESTS_SRC=$(wildcard tests/*.c)
TESTS_OBJ=$(TEST_SRC:tests/%.c=obj/test_%.o)
TESTS=$(TEST_SRC:tests/%.c=tests/test_%)

_ = $(shell mkdir -p obj)

DEBUG ?= undefined

ifneq ($(DEBUG), undefined)
	CPPFLAGS += -DDEBUG=$(DEBUG)
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

HDR = $(wildcard src/*.h) $(wildcard include/*.h)
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=obj/%.o)
BIN = libschedule.so

TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(TEST_SRC:tests/%.c=obj/%.o)
TEST_BIN = $(TEST_SRC:%.c=%)

.SECONDARY: $(OBJ) $(TESTS_OBJ)

.PHONY: all clean tests

all: $(BIN)

tests: $(TESTS)

$(BIN): $(OBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^ $(LIBS)

obj/%.o: src/%.c $(HDR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

tests/test_%: obj/test_%.o $(BIN)
	$(LD) $(TEST_LDFLAGS) -o $@ $< $(TEST_LIBS)

obj/test_%.o: tests/%.c $(HDR)
	$(CC) $(TEST_CPPFLAGS) $(TEST_CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(TESTS_OBJ) $(BIN) $(TESTS)
