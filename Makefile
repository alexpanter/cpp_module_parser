GCC=gcc -Og -std=gnu11 -Wall -Werror -pedantic

.PHONY: clean

all: io_helper main

main: main.c mockup_files.h
	$(GCC) $< -o $@

io_helper: io_helper.c io_helper.h
	$(GCC) -c $< -o $@.o

test_io_helper: test_io_helper.c io_helper
	$(GCC) $< io_helper.o -o $@

clean:
	rm -f main
	rm -f *.o

