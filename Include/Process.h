#pragma once

#include "Memory.h"

#include <cstdint> // std::uint32_t
#include <optional>
#include <string>
#include <string_view>

namespace Diaxx
{
	class Process
	{
	public:
		Process(std::wstring_view name);
		~Process() = default;

		// Prevent copying
		Process(const Process&)            = delete;
		Process& operator=(const Process&) = delete;

		// Prevent moving
		Process(Process&&)            = delete;
		Process& operator=(Process&&) = delete;

		[[nodiscard]] std::wstring_view getName()        const noexcept;
		[[nodiscard]] std::uint32_t     getIdentifier()  const noexcept;
		[[nodiscard]] std::uintptr_t    getBaseAddress() const noexcept;

	private:
		[[nodiscard]] bool getNameAndIdentifier(std::wstring_view name);
		[[nodiscard]] bool getBaseAddress();
		void showInformation() const noexcept;

		std::wstring          m_name        {};
		std::uint32_t         m_identifier  {};
		std::uintptr_t        m_baseAddress {};
		std::optional<Memory> m_memory      {};
	};
}