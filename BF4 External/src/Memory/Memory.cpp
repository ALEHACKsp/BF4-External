#include <Windows.h>
#include "memory.hpp"
#include "../Misc/Logger.hpp"


Memory::~Memory()
{
	this->Detach();
}

/* Gets Process ID */
bool Memory::Attach()
{
	this->process_id = this->GetProcessID("bf4.exe");


	if (this->process_id == 0)
	{
		return false;
	}

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, this->process_id);

	if (handle == INVALID_HANDLE_VALUE)
		return false;

	SetConsoleTitleA("BF4 External aura#0240");

	this->BF4_HANDLE = handle;

	this->module_address = GetModuleBase("bf4.exe");

	
	Logger::Print("BF4.exe ID [%d]", this->process_id);
	Logger::Print("BF4.exe Module Base [0x%llX]", this->module_address);
	return true;

}

bool Memory::Detach()
{
	CloseHandle(this->BF4_HANDLE);
	this->BF4_HANDLE = INVALID_HANDLE_VALUE;
	return true;
}

uintptr_t Memory::GetProcessID(const char* proc)
{
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);

	do
	{
		if (!strcmp(pEntry.szExeFile, proc))
		{
			process_id = pEntry.th32ProcessID;
			CloseHandle(hProcessId);
		}

	} while (Process32Next(hProcessId, &pEntry));
	return process_id;
}



/* Gets Selected Modules Base Address */
uintptr_t Memory::GetModuleBase(const char* module_name)
{
	uintptr_t modBaseAddr = 0;

	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->process_id);

	if (handle != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(handle, &modEntry))
		{
			do
			{
				if (strstr(modEntry.szModule, module_name))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(handle, &modEntry));
		}
	}
	CloseHandle(handle);
	return modBaseAddr;
}

/* when passing the classes name provide namespace example: ConVar::Graphics */

bool Memory::ReadAddressRaw(uintptr_t address, void* buffer, SIZE_T size)
{
	if (ReadProcessMemory(this->BF4_HANDLE, (void*)address, buffer, size, 0))
	{
		return true;
	}
	return false;
}

std::string Memory::ReadUnicodeString(uintptr_t unity_str_address)
{
	const int size = 50;

	wchar_t wide_char_buffer[size] = {};
	char char_buffer[size] = {};

	this->ReadAddressRaw(unity_str_address, wide_char_buffer, sizeof(wchar_t) * size);

	WideCharToMultiByte(CP_UTF8, 0, wide_char_buffer, size, char_buffer, size, 0, 0);

	return std::string(char_buffer);
}

std::string convertToString(char* a, int size)
{
	int i;
	std::string s = "";
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}

std::string Memory::ReadString(uintptr_t address)
{
	char buffer[32];
	this->ReadAddressRaw(address, &buffer, sizeof(buffer));

	std::string ret = std::string(buffer);

	return ret;
}


std::wstring Memory::StringToWString(const std::string& s)
{
	std::wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

Memory M;