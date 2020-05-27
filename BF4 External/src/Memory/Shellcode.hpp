#pragma once
#include "Memory.hpp"

class Shellcode
{
public:
	std::vector<uint8_t> shellcode;

	void push_back(std::vector<uint8_t> shell)
	{
		shellcode.insert(shellcode.end(), shell.begin(), shell.end());
	}

	template <typename t>
	void push_back(t var) {
		std::vector<uint8_t> _shellcode;
		_shellcode.resize(sizeof t);

		memcpy(_shellcode.data(), &var, sizeof t);

		shellcode.insert(shellcode.end(), _shellcode.begin(), _shellcode.end());
	}
};