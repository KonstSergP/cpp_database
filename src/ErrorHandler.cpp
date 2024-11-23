#include "../include/ErrorHandler.hpp"
#include <iostream>


std::map<TE, std::string> TE_messages = 
{
	{TE::NO_COMMAND,   "Can\'t find command to parse\n"},
	{TE::NOCR_OBJ,     "Can\'t find which object to create\n"},
	{TE::PARSE_ATTR,   "Can\'t get attributes of column\n"},
	{TE::PARSE_CNAME,  "Can\'t get name of column\n"},
	{TE::PARSE_TYPE,   "Can\'t get type of column\n"},
	{TE::PARSE_VAL,    "Error while reading value\n"},
	{TE::PARSE_TNAME,  "Can\'t get name of table\n"},
	{TE::EXP_OPEN,     "Expected ( but not found\n"},
	{TE::PARSE_VALS,   "Can\'t find list of values\n"},
	{TE::NO_DEFAULT,   "Trying to get default value, but it doesn\'t exist\n"},
	{TE::WCNT_VALS,    "Wrong number of values in query\n"},
	{TE::NO_UNIQUE,    "Trying to insert not unique value in unique column\n"},
	{TE::DIFF_COL_NUM, "Tables have different number of columns\n"},
	{TE::DIFF_COL_TYPE,"Columns have different types\n"},
	{TE::UNKNOWN_TYPE, "Column has unknown type\n"},
	{TE::CNEXIST, 	   "Column with this name does not exist\n"}
};


TableException::TableException(TE code)
{
	what_message = TE_messages[code];
}


const char* TableException::what() const noexcept
{
    return what_message.c_str();
}
