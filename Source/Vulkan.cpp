#include "Diaxx/Vulkan.hpp"

#include "Diaxx/Constants.hpp"

#include <algorithm>   // std::ranges
#include <cstring>     // std::strcmp
#include <iostream>    // std::cerr
#include <iterator>    // std::distance
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

	void Vulkan::initializeWindow() noexcept
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
		createInstance();           // 2.1
		setupDebugMessenger();      // 2.2
		pickPhysicalDevice();       // 2.3
		createLogicalDevice();      // 2.4
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
		const auto requiredGLFWExtensions = getGLFWExtensions();
		checkGLFWExtensions(static_cast<std::uint32_t>(requiredGLFWExtensions.size()),
			requiredGLFWExtensions.data());

		// Get the necessary layers for the application
		std::vector<const char*> requiredAppLayers {};
		if (Constants::g_enableValidationLayers)
		{
			requiredAppLayers.assign(Constants::g_validationLayers.begin(),
				Constants::g_validationLayers.end());
		}	
		checkAppLayers(requiredAppLayers);

		// Structure that tells Vulkan which global extensions and validation layers to use
		const vk::InstanceCreateInfo createInfo {
			.pApplicationInfo        = &appInfo,
			.enabledLayerCount       = static_cast<std::uint32_t>(requiredAppLayers.size()),
			.ppEnabledLayerNames     = requiredAppLayers.data(),
			.enabledExtensionCount   = static_cast<std::uint32_t>(requiredGLFWExtensions.size()),
			.ppEnabledExtensionNames = requiredGLFWExtensions.data()
		};

		// Issue the call to create an instance
		m_instance = vk::raii::Instance(m_context, createInfo);
	}

	// Check if the extensions required for the GLFW library are compatible with Vulkan
	void Vulkan::checkGLFWExtensions(std::uint32_t glfwExtensionCount, const auto& glfwExtensions) const
	{
		const auto extensionProperties { m_context.enumerateInstanceExtensionProperties() };
		for (std::uint32_t i {}; i < glfwExtensionCount; ++i)
		{
			if (std::ranges::none_of(extensionProperties,
				[glfwExtension = glfwExtensions[i]](const auto& extensionProperty)
				{ return std::strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
			{
				throw std::runtime_error("[Error]: The " + std::string(glfwExtensions[i]) +
					" extension required by GLFW is not supported.");
			}
		}

		std::println("[Debug]: List of supported Vulkan instance extensions:");
		for (const auto& supportedVulkanExtension : extensionProperties)
			std::println("\t- {}", std::string_view(supportedVulkanExtension.extensionName));

		std::println("\n[Debug]: List of extensions required by the GLFW library:");
		for (std::uint32_t i {}; i < glfwExtensionCount; ++i)
			std::println("\t- {}", glfwExtensions[i]);
	}

	// Check if the layers required for the app are compatible with Vulkan
	void Vulkan::checkAppLayers(const auto& appLayers) const
	{
		const auto layerProperties { m_context.enumerateInstanceLayerProperties() };
		if (std::ranges::any_of(appLayers,
			[&layerProperties](const auto& requiredLayer)
			{
				return std::ranges::none_of(layerProperties,
					[requiredLayer](const auto& layerProperty)
					{ return std::strcmp(layerProperty.layerName, requiredLayer) == 0; });
			}))
		{
			throw std::runtime_error("[Error]: One or more required layers are not supported.");
		}

		std::println("\n[Debug]: List of supported Vulkan layers:");
		for (const auto& supportedVulkanLayer : layerProperties)
			std::println("\t- {}", std::string_view(supportedVulkanLayer.layerName));

		std::println("\n[Debug]: List of layers required by the app:");
		for (const auto& requiredAppLayer : appLayers)
			std::println("\t- {}", requiredAppLayer);
	}

	// Return the list of extensions based on whether validation layers are enabled or not
	std::vector<const char*> Vulkan::getGLFWExtensions() const noexcept
	{
		std::uint32_t glfwExtensionCount {};
		const auto glfwExtensions { glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

		std::vector updatedGLFWExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (Constants::g_enableValidationLayers)
		{
			// "VK_EXT_debug_utils"
			updatedGLFWExtensions.push_back(vk::EXTDebugUtilsExtensionName);
		}
			
		return updatedGLFWExtensions;
	}

	// Custom logging function that Vulkan calls when it detects something worth telling
	VKAPI_ATTR vk::Bool32 VKAPI_CALL Vulkan::debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT, vk::DebugUtilsMessageTypeFlagsEXT,
		const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void*) noexcept
	{
		std::cerr << "[Validation Layer]: " << callbackData->pMessage << '\n';

		return vk::False;
	}

	// Connects the function debugCallback to the Vulkan API so you can see validation messages
	void Vulkan::setupDebugMessenger() noexcept
	{
		if (!Constants::g_enableValidationLayers) return;

		constexpr vk::DebugUtilsMessageSeverityFlagsEXT messageSeverity {
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
		};

		constexpr vk::DebugUtilsMessageTypeFlagsEXT messageType {
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		};

		constexpr vk::DebugUtilsMessengerCreateInfoEXT callbackData {
			.messageSeverity = messageSeverity,
			.messageType     = messageType,
			.pfnUserCallback = &debugCallback
		};

		m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(callbackData);
	}

	// Look for and select a graphics card in the system that supports the features we need
	void Vulkan::pickPhysicalDevice()
	{
		const auto devices { m_instance.enumeratePhysicalDevices() };
		if (devices.empty())
			throw std::runtime_error("[Error]: Failed to find GPUs with Vulkan support.");

		const auto deviceIterator { std::ranges::find_if(devices,
			[&](const auto& device)
			{
				// Every operation in Vulkan requires commands to be submitted to a queue
				// There are different types of queues from different queue families
				// Each family of queues allows only a subset of commands
				const auto queueFamilies { device.getQueueFamilyProperties() };
				bool isSuitable { device.getProperties().apiVersion >= VK_API_VERSION_1_3 };

				// Queue Family Properties
				const auto qfpIterator { std::ranges::find_if(queueFamilies,
					[](const auto& qfp)
					{ return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
						static_cast<vk::QueueFlags>(0); }) };

				isSuitable = isSuitable && (qfpIterator != queueFamilies.end());
				const auto extensions { device.enumerateDeviceExtensionProperties() };
				bool found { true };

				// Compare the GPU extensions required for our app with the ones of each GPU
				for (const auto& appExtension : Constants::g_deviceExtensions)
				{
					const auto extensionIterator { std::ranges::find_if(extensions,
						[appExtension](const auto& deviceExtension)
						{ return std::strcmp(deviceExtension.extensionName, appExtension) == 0; }) };

					found = found && extensionIterator != extensions.end();
				}

				isSuitable = isSuitable && found;
				if (isSuitable)
				{
					m_physicalDevice = vk::raii::PhysicalDevice(device);

					const std::uint32_t queueFamilyIndex { static_cast<std::uint32_t>(
						std::distance(queueFamilies.begin(), qfpIterator)) };
					m_graphicsQueueFamilyIndex = queueFamilyIndex;
				}
					
				return isSuitable;
			}) };

		if (deviceIterator == devices.end())
			throw std::runtime_error("[Error]: Failed to find a suitable GPU.");
	}

	// After selecting a physical device we need a logical device to interface with it
	void Vulkan::createLogicalDevice() noexcept
	{
		// Assign priorities to queues between 0.0 and 1.0
		constexpr float queuePriority { 0.0f };
		const vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
			.queueFamilyIndex = m_graphicsQueueFamilyIndex,
			.queueCount       = 1,
			.pQueuePriorities = &queuePriority
		};

		// Structure chaining to enable multiple sets of features
		const vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain {
				{},
				{ .dynamicRendering     = true },
				{ .extendedDynamicState = true }
		};

		// Creation of the logical device
		const vk::DeviceCreateInfo deviceCreateInfo {
			.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount    = 1,
			.pQueueCreateInfos       = &deviceQueueCreateInfo,
			.enabledExtensionCount   = static_cast<std::uint32_t>(Constants::g_deviceExtensions.size()),
			.ppEnabledExtensionNames = Constants::g_deviceExtensions.data()
		};

		m_device        = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
		m_graphicsQueue = vk::raii::Queue(m_device, m_graphicsQueueFamilyIndex, 0);
	}

	void Vulkan::mainLoop()
	{
		while (!glfwWindowShouldClose(m_window))
			glfwPollEvents();
	}

	void Vulkan::cleanup() noexcept
	{
		glfwDestroyWindow(m_window);

		glfwTerminate();
	}
}