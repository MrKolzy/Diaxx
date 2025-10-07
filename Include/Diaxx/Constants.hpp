#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <array>

namespace Constants
{
	inline constexpr int g_width  { 800 };
	inline constexpr int g_height { 600 };

	inline constexpr std::array g_validationLayers { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
	inline constexpr bool g_enableValidationLayers { false };
#else
	inline constexpr bool g_enableValidationLayers { true };
#endif

	inline constexpr std::array g_deviceExtensions { vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName, vk::KHRSynchronization2ExtensionName,
		vk::KHRCreateRenderpass2ExtensionName };
}