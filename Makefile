GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic
APP=cmop

.PHONY: clean

all: io_helper main

$(APP): src/main.c
	$(GCC) $< -o bin/$@

dependency_list: src/dependency_list.c src/dependency_list.h
	$(GCC) -c $< -o bin/$@.o

module_unit.o: src/module_unit.c src/module_unit.h
	$(GCC) -c $< -o bin/$@

io_helper.o: src/io_helper.c src/io_helper.h
	$(GCC) -c $< -o bin/$@

test_io_helper: src/test_io_helper.c io_helper.o module_unit.o
	$(GCC) $< bin/*.o -o bin/$@

clean:
	rm -rf bin/*
	rm -f main
	rm -f *.o

