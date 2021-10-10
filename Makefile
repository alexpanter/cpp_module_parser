MAKEFLAGS += --no-print-directory
.PHONY: clean
APP=cmop

GCC=gcc -g -std=gnu11 -Wall -Werror -pedantic

all:
	@cd src/ && make all

$(APP):
	@cd src/ && make ../bin/$(APP)

test_io_reader:
	@mkdir -p bin
#	@cd src/ && make test_io_reader
	@cd src/ && make ../bin/test_io_reader


clean:
	rm -rf bin/*
	rm -f main
	rm -f *.o

