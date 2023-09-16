CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -ggdb -fsanitize=address -std=c11
LIBS=
OBJECTS=lib.o err.o op.o parser.o vm.o main.o
OUT=stack-vm.out
ARGS=

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LIBS)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY:
clean:
	rm -rfv $(OUT) $(OBJECTS)

.PHONY: run
run: $(OUT)
	./$^ $(ARGS)
