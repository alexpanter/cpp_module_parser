GCC=gcc -std=gnu11 -Wall -Werror -pedantic

.PHONY: clean

main: main.c mockup_files.h
	$(GCC) $< -o $@

clean:
	rm -f main
