#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

#include <cstdint> // std::uint32_t

namespace Diaxx
{
	class Vulkan
	{
	public:
		Vulkan()  = default;
		~Vulkan() = default;

		void run();

	private:
		void initializeWindow(); // 1.0

		void initializeVulkan(); // 2.0

		void createInstance();   // 2.1
		void checkExtensions(std::uint32_t glfwExtensionCount, const auto& glfwExtensions);

		void mainLoop();
		void cleanup();

		GLFWwindow*        m_window   {};
		vk::raii::Context  m_context  {};
		vk::raii::Instance m_instance { nullptr };
	};
}