#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

#include <cstdint> // std::uint32_t
#include <vector>

namespace Diaxx
{
	class Vulkan
	{
	public:
		Vulkan()  = default;
		~Vulkan() = default;

		// Delete the copy constructor and the copy assignment operator
		Vulkan(const Vulkan& vulkan)            = delete;
		Vulkan& operator=(const Vulkan& vulkan) = delete;

		// Delete the move constructor and the move assignment operator
		Vulkan(Vulkan&& vulkan)            = delete;
		Vulkan& operator=(Vulkan&& vulkan) = delete;

		void run();

	private:
		void initializeWindow() noexcept;    // 1.0

		void initializeVulkan();             // 2.0

		void createInstance();               // 2.1
		void checkGLFWExtensions(std::uint32_t glfwExtensionCount, const auto& glfwExtensions) const;
		void checkAppLayers(const auto& appLayers) const;
		[[nodiscard]] std::vector<const char*> getGLFWExtensions() const noexcept;
		[[nodiscard]] static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) noexcept;

		void setupDebugMessenger() noexcept; // 2.2

		void createSurface();                // 2.3

		void pickPhysicalDevice();           // 2.4

		void createLogicalDevice();          // 2.5

		void createSwapChain() noexcept;     // 2.6
		[[nodiscard]] vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<vk::SurfaceFormatKHR>& availableFormats) const noexcept;
		[[nodiscard]] vk::PresentModeKHR chooseSwapPresentationMode(
			const std::vector<vk::PresentModeKHR>& availablePresentationModes) const noexcept;
		[[nodiscard]] vk::Extent2D chooseSwapExtent(
			const vk::SurfaceCapabilitiesKHR& capabilities) const noexcept;

		void mainLoop();
		void cleanup() noexcept;

		GLFWwindow*                      m_window                       {};
		vk::raii::Context                m_context                      {};
		vk::raii::Instance               m_instance                     { nullptr };
		vk::raii::DebugUtilsMessengerEXT m_debugMessenger               { nullptr };
		vk::raii::PhysicalDevice         m_physicalDevice               { nullptr };
		vk::raii::Device                 m_device                       { nullptr };
		std::uint32_t                    m_graphicsQueueFamilyIndex     {};
		vk::PhysicalDeviceFeatures       m_deviceFeatures               {};
		vk::raii::Queue                  m_graphicsQueue                { nullptr };
		vk::raii::SurfaceKHR             m_surface                      { nullptr };
		std::uint32_t                    m_presentationQueueFamilyIndex {};
		vk::raii::Queue                  m_presentationQueue            { nullptr };
		vk::raii::SwapchainKHR           m_swapChain                    { nullptr };
		std::vector<vk::Image>           m_swapChainImages              {};
		vk::Format                       m_swapChainImageFormat         { vk::Format::eUndefined };
		vk::Extent2D                     m_swapChainExtent              {};
	};
}