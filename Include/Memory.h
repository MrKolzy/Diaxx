#pragma once

#include <wil/resource.h>

#include <cstddef> // std::size_t
#include <cstdint> // std::uint32_t
#include <cstdio>  // stderr
#include <print>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Diaxx
{
	class Memory
	{
	public:
		Memory(std::uintptr_t processIdentifier) noexcept;
		~Memory() = default;

		// Prevent copying
		Memory(const Memory&)            = delete;
		Memory& operator=(const Memory&) = delete;

		// Prevent moving
		Memory(Memory&&)            = delete;
		Memory& operator=(Memory&&) = delete;

		template <typename T>
		static T read(std::uintptr_t address)              noexcept;
		template <typename T>
		static bool write(T value, std::uintptr_t address) noexcept;

	private:
		static inline wil::unique_handle m_process {};
	};

	template<typename T>
	inline T Memory::read(std::uintptr_t address) noexcept
	{
		T value {};
		std::size_t bytesRead {};

		const BOOL result { ReadProcessMemory(m_process.get(),
			reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead) };

		if (!result || bytesRead != sizeof(T))
		{
			std::println(stderr, "[Error]: ReadProcessMemory");
			return {};
		}

		return value;
	}

	template<typename T>
	inline bool Memory::write(T value, std::uintptr_t address) noexcept
	{
		std::size_t bytesWritten {};

		const BOOL result { WriteProcessMemory(m_process.get(),
			reinterpret_cast<LPVOID>(address), &value, sizeof(T), &bytesWritten) };

		if (!result || bytesWritten != sizeof(T))
		{
			std::println(stderr, "[Error]: WriteProcessMemory");
			return false;
		}

		return true;
	}
}