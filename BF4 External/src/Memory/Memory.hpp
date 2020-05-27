#pragma once
#include <Windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <sstream>
#include <array>
#include <algorithm>
#include <vector>

class Memory
{
public:
	HANDLE BF4_HANDLE = NULL;
	unsigned int process_id = 0;
	uintptr_t module_address = 0x0;
	uintptr_t gdi32_address = 0x0;
	~Memory();

	bool Attach();

	bool Detach();

	uintptr_t GetProcessID(const char* proc);

	uintptr_t GetModuleBase(const char* module_name);

	bool ReadAddressRaw(uintptr_t address, void* buffer, SIZE_T size);

	std::string ReadUnicodeString(uintptr_t unity_str_address);

	std::string ReadString(uintptr_t address, const unsigned int size);

	std::string ReadString(uintptr_t address, int size);

	std::string ReadString(uintptr_t address, char buffer[]);

	std::string ReadString(uintptr_t address);

	std::wstring StringToWString(const std::string& s);


	template<typename type>
	type Read(uintptr_t address)
	{
		type buffer;
		ReadProcessMemory(this->BF4_HANDLE, (LPCVOID)address, &buffer, sizeof(buffer), 0);
		return buffer;
	}


	template<typename type>
	type ReadChain(uintptr_t address, std::vector<uintptr_t> chain)
	{
		uintptr_t current = address;
		for (int i = 0; i < chain.size() - 1; i++)
		{
			current = this->Read<uintptr_t>(current + chain[i]);
		}
		return this->Read<type>(current + chain[chain.size() - 1]);
	}


	template<typename type>
	type Write(uintptr_t address, type to_write)
	{
		type buffer = to_write;
		WriteProcessMemory(this->BF4_HANDLE, (void*)address, &buffer, sizeof(buffer), 0);
		return 0x0;

	}

	template <class dataType>
	bool WPM(dataType value, DWORD addy)
	{
		return WriteProcessMemory(this->BF4_HANDLE, (PVOID)addy, &value, sizeof(dataType), 0);
	}

};


extern Memory M;