#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

#define SET_TEXT_COLOUR(colour) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colour);

namespace Logger
{
	inline int theme_colour = 4;
	inline int original_colour = 7;
	inline const char* prefix = " [*] ";

	inline void Title(const std::string& str)
	{
		SET_TEXT_COLOUR(theme_colour);
		printf(" --- ");
		SET_TEXT_COLOUR(original_colour);
		printf((str).c_str());
		SET_TEXT_COLOUR(theme_colour);
		printf(" --- \n");
		SET_TEXT_COLOUR(original_colour);

	}

	template <typename ... print_args>
	inline void Print(const std::string& str, print_args ...arguments)
	{
		SET_TEXT_COLOUR(theme_colour);
		printf(prefix);
		SET_TEXT_COLOUR(original_colour);
		printf((str + "\n").c_str(), arguments...);

	}

	template <typename ... print_custom_prefix_args>
	inline void PrintCustomPrefix(const std::string& new_prefix, const std::string& str, print_custom_prefix_args ...arguments)
	{
		SET_TEXT_COLOUR(theme_colour);
		printf(new_prefix.c_str());
		SET_TEXT_COLOUR(original_colour);
		printf((str + "\n").c_str(), arguments...);

	}

	template <typename ... print_update_args>
	inline void PrintUpdate(const std::string& str, print_update_args ...arguments)
	{
		SET_TEXT_COLOUR(theme_colour);
		printf(prefix);
		SET_TEXT_COLOUR(original_colour);
		printf((str + "\r").c_str(), arguments...);
	}

	inline void Flush()
	{
		printf("\n");
	}

	inline inline void Break()
	{
		SET_TEXT_COLOUR(theme_colour);
		printf("----------------------------------------------------");
		SET_TEXT_COLOUR(original_colour);

	}
}