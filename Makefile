MAKEFLAGS += --no-print-directory
.PHONY: clean

GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic

all: io_helper main

$(APP): src/main.c
	$(GCC) $< -o bin/$@

test_io_reader:
	@mkdir -p bin
	@cd src/ && make ../bin/test_io_reader


clean:
	rm -rf bin/*
	rm -f main
	rm -f *.o

