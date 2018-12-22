CC = gcc
CPPFLAGS = -D_GNU_SOURCE
CFLAGS = -W -Wall -Wextra -Werror -std=c99 -fPIC

LD = gcc
LDFLAGS =
LIBS = 

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

.SECONDARY : $(OBJ)

.PHONY : all clean tests

all : $(BIN)

$(BIN) : $(OBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^ $(LIBS)

obj/%.o : src/%.c $(HDR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

tests : $(OBJ)
	@for test in $(wildcard tests/*.c); do \
		obj=$${test%.c}.o ; \
		echo $(CC) $(CPPFLAGS) -Isrc/ $(CFLAGS) -o $$obj -c $$test ; \
		$(CC) $(CPPFLAGS) -Isrc/ $(CFLAGS) -o $$obj -c $$test ; \
		echo $(LD) $(LDFLAGS) -o $${test%.c} $$obj $(OBJ) $(LIBS) ; \
		$(LD) $(LDFLAGS) -o $${test%.c} $$obj $(OBJ) $(LIBS) ; \
	done

clean :
	rm -f $(OBJ) $(BIN)
