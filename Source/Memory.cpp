#include "Memory.h"

namespace Diaxx
{
	Memory::Memory(std::uintptr_t processIdentifier)
	{
		m_process.reset(OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE |
			PROCESS_VM_OPERATION, 0, static_cast<DWORD>(processIdentifier)));
		if (!m_process)
			throw std::runtime_error("The process could not be opened");
	}
}