#include "../include/utils.hpp"


const char* TypeNames[] = {"integer", "boolean", "string", "bytes"};

char hex_char(char symbol)
{
	if ((symbol > 96) && (symbol < 123))
	{
		symbol -= 32;
	}
	if ((symbol >= '0') && (symbol <= '9'))
	{
		return symbol - '0';
	}
	return symbol - 'A';
}


std::string convert_hex_to_char(std::string& hex)
{
	char result[(hex.size()+1)/2];

	if (hex.size() % 2)
	{
		result[0] = hex_char(hex[0]);
	}
	else
	{
		result[0] = 16*hex_char(hex[0]) + hex_char(hex[1]);
	}

	for (unsigned long i = 1; i < (hex.size()/2); i += 1)
	{
		result[i] = 16*hex_char(hex[2*i]) + hex_char(hex[2*i+1]);
	}

	return std::string(result);
}


bool TypeInfo::operator==(TypeInfo& other)
{
	return (type == other.type) && (length == other.length);
}

bool TypeInfo::operator!=(TypeInfo& other)
{
	return !(*this == other);
}

