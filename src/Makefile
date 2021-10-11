GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic
APP=cmop
BIN=../bin

.PHONY: clean

all: $(BIN)/test_io_reader $(BIN)/$(APP)

$(BIN)/module_unit.o: module_unit.c module_unit.h
	$(GCC) -c $< -o $@

$(BIN)/string_helper.o: string_helper.c
	$(GCC) -c $< -o $@

$(BIN)/io_reader.o: io_reader.c io_reader.h
	$(GCC) -c $< -o $@

objects: $(BIN)/module_unit.o $(BIN)/io_reader.o $(BIN)/string_helper.o

#test_io_reader: $(BIN)/test_io_reader

$(BIN)/test_io_reader: test_io_reader.c objects
	$(GCC) $< $(BIN)/*.o -o $@

$(BIN)/$(APP): main.c objects
	$(GCC) $< $(BIN)/*.o -o $@

clean:
	rm -rf bin/*
	rm -f main
	rm -f *.o

