#pragma once

#include <iostream>
#include <cstdio>

#include "../app/global.hpp"

class logger
{
public:
	static void log(const std::string& type, const std::string& text)
	{
		std::cout << "[ " << type << " ] " << text << '\n';
	}

	static void log_info(const std::string& text)
	{
		log("INFO", text);
	}

	static void log_error(const std::string& text)
	{
		log("ERROR", text);
	}

	static void log_warning(const std::string& text)
	{
		log("WARNING", text);
	}

	static void log_debug(const std::string& text)
	{
#ifdef DEBUG
		log("DEBUG", text);
#endif
	}

	static std::string va(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char result[512]{};
		std::vsprintf(result, fmt, va);
		return std::string(result);
	}

#ifndef HELPER
	static std::string get_toggle(bool input)
	{
		switch (input)
		{
		case true:
			return "On";
			break;

		case false:
			return "Off";
			break;

		default:
			return "???";
			break;
		}
	}

	static std::vector<std::string> split(const std::string& s, const std::string& seperator)
	{
		std::vector<std::string> output;

		std::string::size_type prev_pos = 0, pos = 0;

		while ((pos = s.find(seperator, pos)) != std::string::npos)
		{
			std::string substring(s.substr(prev_pos, pos - prev_pos));

			output.push_back(substring);

			prev_pos = ++pos;
		}

		output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

		return output;
	}

	static void to_lower(std::string& string)
	{
		std::for_each(string.begin(), string.end(), ([](char& c)
		{
			c = std::tolower(c);
		}));
	}

	static void to_upper(std::string& string)
	{
		std::for_each(string.begin(), string.end(), ([](char& c)
		{
			c = std::toupper(c);
		}));
	}

	static void remove_non_ascii(std::string& s)
	{
		s.erase(std::remove_if(s.begin(), s.end(), [](char c) {return !(c >= 0 && c < 128); }), s.end());
	}

	static std::string convert_codepage(std::string& in)
	{
		std::vector<wchar_t> wide;
		std::vector<char> normal;
		const int max_size = in.size() * 4 + 1;

		wide.resize(max_size);
		normal.resize(max_size);

		WideCharToMultiByte (
			CP_UTF8,
			0,
			&wide[0],
			MultiByteToWideChar(1252, 0, in.c_str(), in.size(), &wide[0], wide.size()), &normal[0], normal.size(),
			nullptr,
			nullptr
		);

		return &normal[0];
	}

	static void rem_path_info(std::string& str, const std::string& dir)
	{
		//Assuming ext is 4 for now
		str.erase(0, dir.size() + 1).erase(str.size() - 4, 4);
	}
#endif
};