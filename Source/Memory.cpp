#include "Memory.h"

#include <stdexcept>

namespace Diaxx
{
	Memory::Memory(std::uint32_t processIdentifier)
	{
		constexpr DWORD desiredAccess { PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION };
		m_process.reset(OpenProcess(desiredAccess, FALSE, static_cast<DWORD>(processIdentifier)));
		if (!m_process)
			throw std::runtime_error("The process could not be opened");
	}
}