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
		if (getNameAndIdentifier(name) == 0)
			throw std::runtime_error("The process could not be found");
	}

	bool Process::getNameAndIdentifier(std::wstring_view name)
	{
		bool isFound { false };

		const HANDLE processSnapshot { CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
		if (processSnapshot == INVALID_HANDLE_VALUE)
		{
			std::println(stderr, "[Error]: CreateToolhelp32Snapshot");
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
			if (_wcsicmp(name.data(), processEntry.szExeFile) == 0)
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
}