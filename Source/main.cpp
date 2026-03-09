#include "Memory.h"
#include "Process.h"

#include <cstdio>  // stderr
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <exception>
#include <print>

int main()
{
	try
	{
		const Diaxx::Process process { L"Ac_ClIeNt.ExE" };
	}
	catch (const std::exception& exception)
	{
		std::println(stderr, "[Exception]: {}", exception.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}