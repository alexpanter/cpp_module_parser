MAKEFLAGS += --no-print-directory
.PHONY: clean bindir
APP=cmop

all: bindir
	@cd src/ && make all

bindir:
	@mkdir -p bin

$(APP): bindir
	@cd src/ && make ../bin/$(APP)

test_io_reader: bindir
	@cd src/ && make ../bin/test_io_reader

clean:
	rm -rf bin/
