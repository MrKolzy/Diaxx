#include "Process.h"

#include <cstdio>     // stderr
#include <print>
#include <stdexcept>  // std::runtime_error

#include <Windows.h>

#include <TlHelp32.h> // CreateToolhelp32Snapshot

namespace Diaxx
{
	Process::Process(std::wstring_view name)
	{
		if (!getNameAndIdentifier(name) || !getBaseAddress())
			throw std::runtime_error("The process could not be found");
	}

	std::wstring_view Process::getName() const noexcept
	{
		return m_name;
	}

	std::uint32_t Process::getIdentifier() const noexcept
	{
		return m_identifier;
	}

	std::uintptr_t Process::getBaseAddress() const noexcept
	{
		return m_baseAddress;
	}

	bool Process::getNameAndIdentifier(std::wstring_view name)
	{
		bool isFound { false };

		const HANDLE processSnapshot { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
		if (processSnapshot == INVALID_HANDLE_VALUE)
		{
			std::println(stderr, "[Error]: CreateToolhelp32Snapshot (Process)");
			return false;
		}

		PROCESSENTRY32 processEntry { .dwSize { sizeof(PROCESSENTRY32) } };

		if (!Process32First(processSnapshot, &processEntry))
		{
			std::println(stderr, "[Error]: Process32First");
			CloseHandle(processSnapshot);
			return false;
		}
			
		do
		{
			if (!_wcsicmp(name.data(), processEntry.szExeFile))
			{
				m_name       = processEntry.szExeFile;
				m_identifier = processEntry.th32ProcessID;
				isFound      = true;
				break;
			}
		}
		while (Process32Next(processSnapshot, &processEntry));

		CloseHandle(processSnapshot);
		return isFound;
	}

	bool Process::getBaseAddress()
	{
		bool isFound { false };

		const HANDLE moduleSnapshot { CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 |
			TH32CS_SNAPMODULE, m_identifier) };
		if (moduleSnapshot == INVALID_HANDLE_VALUE)
		{
			std::println(stderr, "[Error]: CreateToolhelp32Snapshot (Module)");
			std::println(stderr, "[Warning]: Check the process architecture");
			return false;
		}

		MODULEENTRY32 moduleEntry { .dwSize { sizeof(MODULEENTRY32) } };

		if (!Module32First(moduleSnapshot, &moduleEntry))
		{
			std::println(stderr, "[Error]: Module32First");
			CloseHandle(moduleSnapshot);
			return false;
		}

		do
		{
			if (moduleEntry.th32ProcessID == m_identifier)
			{
				m_baseAddress = reinterpret_cast<std::uintptr_t>(moduleEntry.modBaseAddr);
				isFound       = true;
				break;
			}
		}
		while (Module32Next(moduleSnapshot, &moduleEntry));

		CloseHandle(moduleSnapshot);
		return isFound;
	}
}