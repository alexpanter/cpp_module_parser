module;

#include <iostream>

export module car;

export import :door;
export import :engine;
export import :seat;
export import :window;

export namespace car
{
	void build_car()
	{
		std::cout << "Congratulations, you have built a car!" << std::endl;
	}
}
