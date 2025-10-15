#include "Diaxx/Vulkan.hpp"

#include "Diaxx/Constants.hpp"

#include <algorithm> // std::ranges
#include <array>
#include <cstring>   // std::strcmp
#include <iostream>  // std::cerr
#include <iterator>  // std::distance
#include <limits>    // std::numeric_limits 
#include <print>       
#include <stdexcept> // std::runtime_error
#include <string>    
#include <string_view>

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

		// Issue the call to create a window
		m_window = glfwCreateWindow(Constants::g_width, Constants::g_height,
			"Diaxx", nullptr, nullptr);
	}

	void Vulkan::initializeVulkan() // 2.0
	{
		createInstance();           // 2.1
		setupDebugMessenger();      // 2.2
		createSurface();            // 2.3
		pickPhysicalDevice();       // 2.4
		createLogicalDevice();      // 2.5
		createSwapChain();          // 2.6
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
		std::cerr << "\n[Validation Layer]: " << callbackData->pMessage << '\n';

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

		// Issue the call to create a debugMessenger
		m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(callbackData);
	}

	// Connection between Vulkan and the window system to present results to the screen
	void Vulkan::createSurface()
	{
		VkSurfaceKHR surface {};
		if (glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surface) != 0)
			throw std::runtime_error("[Error]: Failed to create window surface.");

		// Issue the call to create a surface
		m_surface = vk::raii::SurfaceKHR(m_instance, surface);
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
					[&](const auto& qfp)
					{
						const bool graphicsSupport {
							(qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
							static_cast<vk::QueueFlags>(0) };

						const std::uint32_t index {
							static_cast<std::uint32_t>(&qfp - queueFamilies.data()) };

						// Check if the queue family supports presentation using the index
						const VkBool32 presentationSupport {
							device.getSurfaceSupportKHR(index, *m_surface) };
						if (presentationSupport)
							m_presentationQueueFamilyIndex = index;

						return graphicsSupport;
					}) };

				// If the queue family supports graphics then it's suitable
				isSuitable = isSuitable && (qfpIterator != queueFamilies.end());
				const auto extensions { device.enumerateDeviceExtensionProperties() };
				bool found { true };

				// Compare the GPU extensions required for our app with the ones of the current GPU
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
					// Issue the call to create a physical device
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
	void Vulkan::createLogicalDevice()
	{
		const std::vector<vk::QueueFamilyProperties> queueFamilyProperties {
			m_physicalDevice.getQueueFamilyProperties() };

		// Find the queue family that supports graphics
		const auto graphicsQueueFamilyProperty { std::ranges::find_if(queueFamilyProperties,
			[](const auto& qfp)
			{ return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) !=
				static_cast<vk::QueueFlags>(0); }) };

		auto graphicsIndex { static_cast<std::uint32_t>(
			std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty)) };
		
		// Check if the queue family also supports presentation using the index
		auto presentationIndex { m_physicalDevice.getSurfaceSupportKHR(graphicsIndex, *m_surface) ?
			graphicsIndex : static_cast<std::uint32_t>(queueFamilyProperties.size()) };

		// The queue family doesn't support presentation
		if (presentationIndex == queueFamilyProperties.size())
		{
			for (std::size_t i {}; i < queueFamilyProperties.size(); ++i)
			{
				// Look for another queue family that supports both graphics and presentation
				if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
					m_physicalDevice.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), *m_surface))
				{
					graphicsIndex     = static_cast<std::uint32_t>(i);
					presentationIndex = graphicsIndex;
					break;
				}
			}

			// If no queue family supporting graphics and presentation has been found
			if (presentationIndex == queueFamilyProperties.size())
			{
				// Find one queue family that supports only presentation
				for (std::size_t i {}; i < queueFamilyProperties.size(); ++i)
				{
					if (m_physicalDevice.getSurfaceSupportKHR(
							static_cast<std::uint32_t>(i), *m_surface))
					{
						presentationIndex = static_cast<std::uint32_t>(i);
						break;
					}
				}
			}
		}

		if ((graphicsIndex == queueFamilyProperties.size()) ||
			(presentationIndex == queueFamilyProperties.size()))
		{
			throw std::runtime_error("[Error]: Could not find a queue for graphics or presentation.");
		}

		m_graphicsQueueFamilyIndex     = graphicsIndex;
		m_presentationQueueFamilyIndex = presentationIndex;

		const auto availableExtensions { m_physicalDevice.enumerateDeviceExtensionProperties() };

		// Extension required to display images directly on the screen
		const bool swapChainSupported { std::ranges::any_of(availableExtensions,
			[](const auto& extension)
			{ return std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0; }) };

		if (!swapChainSupported)
			throw std::runtime_error("[Error]: Physical device doesn't support VK_KHR_swapchain.");

		// Vector for storing one or more device queue create infos
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};

		// Assign priorities to queues between 0.0 and 1.0
		constexpr float queuePriority { 0.0f };
		const vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
			.queueFamilyIndex = m_graphicsQueueFamilyIndex,
			.queueCount       = 1,
			.pQueuePriorities = &queuePriority
		};
		queueCreateInfos.push_back(deviceQueueCreateInfo);

		// If the presentation queue isn't the same as the graphics queue add it separately
		if (m_presentationQueueFamilyIndex != m_graphicsQueueFamilyIndex)
		{
			queueCreateInfos.push_back({
				.queueFamilyIndex = m_presentationQueueFamilyIndex,
				.queueCount       = 1,
				.pQueuePriorities = &queuePriority
			});
		}

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
			.queueCreateInfoCount    = static_cast<std::uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos       = queueCreateInfos.data(),
			.enabledExtensionCount   = static_cast<std::uint32_t>(Constants::g_deviceExtensions.size()),
			.ppEnabledExtensionNames = Constants::g_deviceExtensions.data()
		};

		// Issue the call to create a device, graphicsQueue and presentationQueue
		m_device            = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
		m_graphicsQueue     = vk::raii::Queue(m_device, m_graphicsQueueFamilyIndex, 0);
		m_presentationQueue = vk::raii::Queue(m_device, m_presentationQueueFamilyIndex, 0);
	}

	// Queue of images waiting to be presented to the screen synchronized with the refresh rate
	void Vulkan::createSwapChain() noexcept
	{
		// VkPhysicalDevice and VkSurfaceKHR are the core components of the swap chain
		const auto surfaceCapabilities { m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface) };

		// Query the supported surface formats
		const vk::SurfaceFormatKHR swapChainSurfaceFormat { chooseSwapSurfaceFormat(
			m_physicalDevice.getSurfaceFormatsKHR(m_surface)) };
		m_swapChainImageFormat = swapChainSurfaceFormat.format;

		const vk::Extent2D swapChainExtent { chooseSwapExtent(surfaceCapabilities) };
		m_swapChainExtent = swapChainExtent;

		auto minImageCount { std::max(3u, surfaceCapabilities.minImageCount) };
		minImageCount = (surfaceCapabilities.maxImageCount > 0 &&
			minImageCount > surfaceCapabilities.maxImageCount) ? surfaceCapabilities.maxImageCount :
			minImageCount;

		// Specify how many images we want in the swap chain, we increase it by one because
		// sometimes we have to wait for the driver to complete internal operations
		std::uint32_t imageCount { surfaceCapabilities.minImageCount + 1 };

		// Make sure to not exceed the maximum number of images while doing this
		if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
			imageCount = surfaceCapabilities.maxImageCount;

		vk::SwapchainCreateInfoKHR swapChainCreateInfo {
			.flags            = vk::SwapchainCreateFlagsKHR(),
			.surface          = m_surface,
			.minImageCount    = minImageCount,
			.imageFormat      = swapChainSurfaceFormat.format,
			.imageColorSpace  = swapChainSurfaceFormat.colorSpace,
			.imageExtent      = swapChainExtent,
			.imageArrayLayers = 1,
			.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.preTransform     = surfaceCapabilities.currentTransform,
			.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			// Query the supported presentation modes
			.presentMode      = chooseSwapPresentationMode(
				m_physicalDevice.getSurfacePresentModesKHR(m_surface)),
			.clipped          = true,
			.oldSwapchain     = nullptr
		};

		const std::array<std::uint32_t, 2> queueFamilyIndices { m_graphicsQueueFamilyIndex,
			m_presentationQueueFamilyIndex };
		
		// Specify how to handle swap chain images that will be used across multiple queue families
		if (m_graphicsQueueFamilyIndex != m_presentationQueueFamilyIndex)
		{
			swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
		}
		else
		{
			swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eExclusive;
			swapChainCreateInfo.queueFamilyIndexCount = 0;
			swapChainCreateInfo.pQueueFamilyIndices   = nullptr;
		}

		// Issue the call to create a swapChain and swapChainImages
		m_swapChain       = vk::raii::SwapchainKHR(m_device, swapChainCreateInfo);
		m_swapChainImages = m_swapChain.getImages();
	}

	// Find the optimal settings for the best possible swap chain
	vk::SurfaceFormatKHR Vulkan::chooseSwapSurfaceFormat(
		const std::vector<vk::SurfaceFormatKHR>& availableFormats) const noexcept
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
				availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) // Color Depth
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	// The most important setting for the swap chain because it represents the actual conditions
	// for showing images to the screen, there are four possible modes available in Vulkan
	vk::PresentModeKHR Vulkan::chooseSwapPresentationMode(
		const std::vector<vk::PresentModeKHR>& availablePresentationModes) const noexcept
	{
		for (const auto& availablePresentationMode : availablePresentationModes)
		{
			if (availablePresentationMode == vk::PresentModeKHR::eMailbox) // Triple Buffering
				return availablePresentationMode;
		}

		return vk::PresentModeKHR::eFifo;
	}

	// Resolution of the swap chain images, it's almost always exactly equal to the resolution of
	// the window that we're drawing to in pixels
	vk::Extent2D Vulkan::chooseSwapExtent(
		const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept
	{
		// Match the resolution of the window by setting the width and height in currentExtent
		if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
			return capabilities.currentExtent;

		int width  {};
		int height {};
		// Vulkan works with pixels so we must use this function to query the resolution of the window
		glfwGetFramebufferSize(m_window, &width, &height);

		// Bound the values between the allowed minimum and maximum extents that are supported
		return {
			std::clamp<std::uint32_t>(static_cast<std::uint32_t>(width),
				capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<std::uint32_t>(static_cast<std::uint32_t>(height),
				capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
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