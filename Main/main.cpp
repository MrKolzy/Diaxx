#include "Diaxx/Vulkan.hpp"

#include <cstdlib>   // EXIT_SUCCESS and EXIT_FAILURE
#include <iostream>  // std::cerr
#include <stdexcept> // std::exception

int main()
{
	Diaxx::Vulkan app {};

	try
	{
		app.run();
	}
	catch (const std::exception& exception)
	{
		std::cerr << "\n[Exception]: " << exception.what() << '\n';

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}