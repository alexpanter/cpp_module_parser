GCC=gcc -Og -std=gnu11 -Wall -Werror -pedantic

.PHONY: clean

all: io_helper main

main: main.c
	$(GCC) $< -o $@

dependency_list: dependency_list.c dependency_list.h
	$(GCC) -c $< -o $@.o

module_unit: module_unit.c module_unit.h
	$(GCC) -c $< -o $@.o

io_helper: io_helper.c io_helper.h
	$(GCC) -c $< -o $@.o

test_io_helper: test_io_helper.c io_helper module_unit
	$(GCC) $< *.o -o $@

clean:
	rm -f main
	rm -f *.o

