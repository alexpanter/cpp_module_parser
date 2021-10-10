module;

#include <iostream>

#ifdef HELLO
const char* get_hello() {
	return "HELLO";
}
#endif

int say_hello() {
	std::cout << get_hello() << '\n';
	return 42;
}
