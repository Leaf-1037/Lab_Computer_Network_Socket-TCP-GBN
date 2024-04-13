#pragma once

#ifndef RR_CONFIG_H_
#define RR_CONFIG_H_
#include <string>
#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <ctime>
#include "http.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <fstream>
#include <map>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <ctime>
#include <iterator>
#include <string.h>

namespace rr
{
	class RrConfig
	{
	public:
		RrConfig()
		{
		}
		~RrConfig()
		{
		}
		bool ReadConfig(const std::string& filename);
		std::string ReadString(const char* section, const char* item, const char* default_value);
		int ReadInt(const char* section, const char* item, const int& default_value);
		float ReadFloat(const char* section, const char* item, const float& default_value);
	private:
		bool IsSpace(char c);
		bool IsCommentChar(char c);
		void Trim(std::string& str);
		bool AnalyseLine(const std::string& line, std::string& section, std::string& key, std::string& value);

	private:
		//std::map<std::string, std::string> settings_;
		std::map<std::string, std::map<std::string, std::string> >settings_;
	};
}
#endif

