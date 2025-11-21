#pragma once

#include <cstdint> // std::uint32_t
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

	private:
		[[nodiscard]] bool getNameAndIdentifier(std::wstring_view name);

		std::wstring  m_name       {};
		std::uint32_t m_identifier {};
	};
}