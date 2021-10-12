GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic
APP=cmop
DEFAPP=-DAPPNAME=\""cmop\""
BIN=../bin

.PHONY: clean

all: objects $(BIN)/test_io_reader $(BIN)/$(APP)

# ~~~ OBJECTS ~~~
OBJS=module_unit.o io_reader.o string_helper.o program_args.o
OBJECTS=$(patsubst %.o, $(BIN)/%.o, $(OBJS))

$(BIN)/module_unit.o: module_unit.c module_unit.h
	$(GCC) -c $< -o $@

$(BIN)/string_helper.o: string_helper.c
	$(GCC) -c $< -o $@

$(BIN)/io_reader.o: io_reader.c io_reader.h
	$(GCC) -c $< -o $@

$(BIN)/program_args.o: program_args.c program_args.h
	$(GCC) -c $< -o $@

objects: $(OBJECTS)

# ~~~ EXECUTABLES ~~~
$(BIN)/test_io_reader: test_io_reader.c objects
	$(GCC) $< $(BIN)/*.o -o $@

$(BIN)/$(APP): main.c objects
	$(GCC) $< $(BIN)/*.o -o $@ $(DEFAPP)

# ~~~ CLEANUP ~~~
clean:
	rm -rf $(BIN)
