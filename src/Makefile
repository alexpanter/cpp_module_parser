GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic
APP=cmop
BIN=../bin

.PHONY: clean

all: io_helper main

$(BIN)/$(APP): main.c
	$(GCC) $< -o $@

$(BIN)/module_unit.o: module_unit.c module_unit.h
	$(GCC) -c $< -o $@

$(BIN)/string_helper.o: string_helper.c string_helper.o
	$(GCC) -c $< -o $@

$(BIN)/io_reader.o: io_reader.c io_reader.h
	$(GCC) -c $< -o $@

objects: $(BIN)/module_unit.o $(BIN)/io_reader.o $(BIN)/string_helper.o

$(BIN)/test_io_reader: test_io_reader.c objects
	$(GCC) $< $(BIN)/*.o -o $@

clean:
	rm -rf bin/*
	rm -f main
	rm -f *.o

