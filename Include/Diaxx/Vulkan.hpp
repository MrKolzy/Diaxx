#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

#include <cstdint> // std::uint32_t
#include <vector>  // std::vector

namespace Diaxx
{
	class Vulkan
	{
	public:
		Vulkan()  = default;
		~Vulkan() = default;

		void run();

	private:
		void initializeWindow();    // 1.0

		void initializeVulkan();    // 2.0

		void createInstance();      // 2.1
		void checkGLFWExtensions(std::uint32_t glfwExtensionCount, const auto& glfwExtensions);
		void checkAppLayers(const auto& appLayers);
		std::vector<const char*> getGLFWExtensions();
		static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

		void setupDebugMessenger(); // 2.2

		void mainLoop();
		void cleanup();

		GLFWwindow*                      m_window         {};
		vk::raii::Context                m_context        {};
		vk::raii::Instance               m_instance       { nullptr };
		vk::raii::DebugUtilsMessengerEXT m_debugMessenger { nullptr };
	};
}