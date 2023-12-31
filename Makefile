CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Wswitch-enum -ggdb -fsanitize=address -std=c11
LIBS=-lm
OBJECTS=src/lib.o src/lexer.o src/data.o src/op.o src/parser.o src/err.o src/vm.o
TEST_OBJECTS=tests/test-lib.o tests/test-op.o tests/test-lexer.o tests/test.o
ARGS=
OUT=

.PHONY: all
all: interpreter.out assembler.out test.out

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LIBS)

assembler.out: $(OBJECTS) src/assembler.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

interpreter.out: $(OBJECTS) src/interpreter.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test.out: $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY: run
run: $(OUT)
	./$^ $(ARGS)

.PHONY: test
test: test.out
	./$^ $(ARGS)

.PHONY:
clean:
	rm -rfv *.o src/*.o tests/*.o tests/*.txt interpreter.out assembler.out test.out
