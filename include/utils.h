#pragma once

#include <string>


#define RED "\033[1;31m"
#define NORMAL "\033[1;0m"


enum Types {Integer, Boolean, Text, Bytes};

extern const char* TypeNames[];

struct TypeInfo
{
	Types type;
	int length;

	bool operator==(TypeInfo& other);
	bool operator!=(TypeInfo& other);

	TypeInfo();
	TypeInfo(Types num);
};

