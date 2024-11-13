#pragma once

#include <string>

enum Types {Integer, Boolean, Text, Bytes};

extern const char* TypeNames[];

struct TypeInfo
{
	Types type;
	int length;

	bool operator==(TypeInfo& other);
	bool operator!=(TypeInfo& other);
};

char hex_char(char symbol);
std::string convert_hex_to_char(std::string& hex);	

