GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic
APP=cmop

.PHONY: clean

all: io_helper main

$(APP): main.c
	$(GCC) $< -o $@

dependency_list: dependency_list.c dependency_list.h
	$(GCC) -c $< -o $@.o

module_unit.o: module_unit.c module_unit.h
	$(GCC) -c $< -o $@

io_helper.o: io_helper.c io_helper.h
	$(GCC) -c $< -o $@

test_io_helper: test_io_helper.c io_helper.o module_unit.o
	$(GCC) $< *.o -o $@

clean:
	rm -f main
	rm -f *.o

