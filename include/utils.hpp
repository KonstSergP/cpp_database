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

	TypeInfo();
	TypeInfo(Types num);
};

