#include "../include/utils.h"


const char* TypeNames[] = {"int32", "boolean", "string", "bytes"};


bool TypeInfo::operator==(TypeInfo& other)
{
	return (type == other.type) && (length == other.length);
}

bool TypeInfo::operator!=(TypeInfo& other)
{
	return !(*this == other);
}


TypeInfo::TypeInfo()
{}


TypeInfo::TypeInfo(Types num)
{
	type = num;
	length = 1;
}
