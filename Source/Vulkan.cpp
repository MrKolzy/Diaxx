#include "Diaxx/Vulkan.hpp"

#include "Diaxx/Constants.hpp"

#include <algorithm>   // std::ranges
#include <cstring>     // std::strcmp
#include <print>       // std::print
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

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

	void Vulkan::initializeVulkan() // 2.0
	{
		createInstance(); // 2.1
	}

	// The instance is the connection between your application and the Vulkan library
	void Vulkan::createInstance()
	{
		// Optional structure with information about our application
		constexpr vk::ApplicationInfo appInfo {
			.pApplicationName   = "Diaxx",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName        = "Diaxx Engine",
			.engineVersion      = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion         = vk::ApiVersion14
		};

		// Get the necessary extensions for the GLFW library
		std::uint32_t glfwExtensionCount {};
		const auto glfwExtensions { glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

		checkExtensions(glfwExtensionCount, glfwExtensions);

		// Structure that tells Vulkan which global extensions and validation layers to use
		const vk::InstanceCreateInfo createInfo {
			.pApplicationInfo        = &appInfo,
			.enabledExtensionCount   = glfwExtensionCount,
			.ppEnabledExtensionNames = glfwExtensions
		};

		// Issue the call to create an instance
		m_instance = vk::raii::Instance(m_context, createInfo);
	}

	// Check if the extensions required for the GLFW library are compatible with Vulkan
	void Vulkan::checkExtensions(std::uint32_t glfwExtensionCount, const auto& glfwExtensions)
	{
		const auto extensionProperties { m_context.enumerateInstanceExtensionProperties() };
		for (std::uint32_t i {}; i < glfwExtensionCount; ++i)
		{
			if (std::ranges::none_of(extensionProperties,
				[glfwExtension = glfwExtensions[i]](const auto& extensionProperty)
				{ return std::strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
			{
				throw std::runtime_error("The " + std::string(glfwExtensions[i]) +
					" extension required by GLFW is not supported.\n");
			}
		}

		std::println("[Debug]: List of supported Vulkan instance extensions:");
		for (const auto& extension : extensionProperties)
			std::println("\t- {}", std::string_view(extension.extensionName));

		std::println("\n[Debug]: List of extensions required by the GLFW library:");
		for (std::uint32_t i {}; i < glfwExtensionCount; ++i)
			std::println("\t- {}", std::string_view(glfwExtensions[i]));
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