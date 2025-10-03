#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Diaxx
{
	class Vulkan
	{
	public:
		Vulkan()  = default;
		~Vulkan() = default;

		void run();

	private:
		void initializeWindow();
		void initializeVulkan();
		void mainLoop();
		void cleanup();

		GLFWwindow* m_window {};
	};
}