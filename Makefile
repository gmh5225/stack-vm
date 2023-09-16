CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Wswitch-enum -ggdb -fsanitize=address -std=c11
LIBS=
OBJECTS=lib.o err.o op.o parser.o vm.o
TEST_OBJECTS=tests/test-lib.o tests/test.o
ARGS=
OUT=

.PHONY: all
all: interpreter.out assembler.out test.out

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LIBS)

assembler.out: $(OBJECTS) assembler.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

interpreter.out: $(OBJECTS) interpreter.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test.out: $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY: run
run: $(OUT)
	./$(OUT) $(ARGS)

.PHONY:
clean:
	rm -rfv $(OUT) $(OBJECTS) $(TEST_OBJECTS) tests/*.txt assembler.out interpreter.out test.out
