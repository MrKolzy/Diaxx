#include "Diaxx/Vulkan.hpp"

#include "Diaxx/Constants.hpp"

namespace Diaxx
{
	void Vulkan::run()
	{
		initializeWindow();
		initializeVulkan();
		mainLoop();
		cleanup();
	}

	void Vulkan::initializeWindow()
	{
		glfwInit();

		// Initialize the GLFW library indicating that it's not an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(Constants::g_width, Constants::g_height,
			"Diaxx", nullptr, nullptr);
	}

	void Vulkan::initializeVulkan()
	{
	}

	void Vulkan::mainLoop()
	{
		while (!glfwWindowShouldClose(m_window))
			glfwPollEvents();
	}

	void Vulkan::cleanup()
	{
		glfwDestroyWindow(m_window);

		glfwTerminate();
	}
}