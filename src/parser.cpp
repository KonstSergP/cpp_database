#include <iostream>
#include <stdlib.h>
#include <regex>
#include <string>
#include <cstdint>

#include "../include/parser.hpp"
#include "../include/utils.hpp"

using namespace Parsing;

bool Parser::parse_pattern(std::regex regexp)
{

	std::cmatch match_result;

	bool success = std::regex_search(pos_, end_,
									match_result, regexp,
									std::regex_constants::match_continuous);

	if (success)
	{
		pos_ = match_result[0].second;

	}

	return success;

}

bool Parser::parse_pattern(std::regex regexp, std::string& ret)
{

	std::cmatch match_result;

	bool success = std::regex_search(pos_, end_,
									match_result, regexp,
									std::regex_constants::match_continuous);

	if (success)
	{
		pos_ = match_result[0].second;

		ret = std::string(match_result[0].first, match_result[0].second);

	}

	return success;

}


bool Parser::parse_space_newline_seq()
{
	std::regex pat("[ \t\n]+");

	return parse_pattern(pat);
}


void Parser::set_query(std::string query)
{
	query_ = query;
	pos_ = query_.c_str();
	end_ = pos_ + query.size();
}

bool Parser::parse_command(std::string& command)
{
	parse_space_newline_seq();
	std::regex pat("CREATE|SELECT|INSERT|UPDATE|DELETE", std::regex::icase);

	bool success = parse_pattern(pat, command);
	std::cout << "parse_command: " << command << "\n";
	return success;
}


bool Parser::parse_name(std::string& name)
{
	parse_space_newline_seq();
	std::regex pat("[a-zA-Z_]+");

	bool success = parse_pattern(pat, name);

	parse_colon();

	return success;
}

bool Parser::parse_creation(std::string& name)
{
	parse_space_newline_seq();
	std::regex pat("TABLE|INDEX", std::regex::icase);

	bool success = parse_pattern(pat, name);
	return success;
}


bool Parser::parse_attributes(int& flags)
{
	flags = 0;
	parse_space_newline_seq();
	if (parse_open_fig())
	{
		std::string attribute;
		while (parse_attr(attribute))
		{
			if (attribute == "key")
			{
				flags |= 5;
			} else if (attribute == "autoincrement")
			{
				flags |= 2;
			} else if (attribute == "unique")
			{
				flags |= 4;
			} else
			{
				printf("parse_attributes erorr\n"); exit(-1);
			}
			parse_comma();
		}
		if (!parse_close_fig()) {printf("error in parse_close_fig\n"); exit(-1);}
	}
	else
	{
		flags = 0;
	}
	std::cout << "Flags: " << flags << "\n";
	return true;
}

bool Parser::parse_open_fig()
{
	parse_space_newline_seq();
	std::regex pat("\\{");
	return parse_pattern(pat);
}

bool Parser::parse_close_fig()
{
	parse_space_newline_seq();
	std::regex pat("\\}");
	return parse_pattern(pat);
}


bool Parser::parse_attr(std::string& attribute)
{
	parse_space_newline_seq();
	std::regex pat("key|autoincrement|unique");

	bool success = parse_pattern(pat, attribute);
	return success;
}


bool Parser::parse_comma()
{
	parse_space_newline_seq();
	std::regex pat(",");
	return parse_pattern(pat);
}

bool Parser::parse_colon()
{
	parse_space_newline_seq();
	std::regex pat(":");
	return parse_pattern(pat);
}

bool Parser::parse_equal_sign()
{
	parse_space_newline_seq();
	std::regex pat("=");
	return parse_pattern(pat);
}


bool Parser::parse_type(TypeInfo& type_info)
{
	parse_space_newline_seq();
	std::string type;
	std::regex pat("int32|bool|string|bytes");
	bool success = parse_pattern(pat, type);
	if (!success) {exit(-1);}

	if (type == "int32")
	{
		type_info.type = Integer;
		type_info.length = 1;
	}
	if (type == "bool")
	{
		type_info.type = Boolean;
		type_info.length = 1;
	}
	int len;
	parse_array_len(len);

	if (type == "string")
	{
		type_info.type = Text;
		type_info.length = len;
	}
	if (type == "bytes")
	{
		type_info.type = Bytes;
		type_info.length = len;
	}

	return success;
}


bool Parser::parse_array_len(int& len)
{
	parse_space_newline_seq();
	parse_pattern(std::regex("\\["));
	bool success = parse_number(len);
	parse_pattern(std::regex("\\]"));

	return success;
}

bool Parser::parse_number(int& number)
{
	parse_space_newline_seq();
	std::regex pat("0|(\\+|-)?[1-9][0-9]*");

	std::string val;

	bool success = parse_pattern(pat, val);
	if (success)
	{
		number = std::stoi(val);
	}

	return success;
}


bool Parser::parse_value(TypeInfo type_info, std::shared_ptr<void>& ptr)
{
	bool success;

	if (!parse_equal_sign()) {return true;}
	std::cout << "Exists default value\n";
	if (type_info.type == Integer)
	{
		std::cout << "AAA\n";
		std::shared_ptr<int> number = std::make_shared<int>();
		success = parse_number(*number);
		ptr = number;
	} else if (type_info.type == Boolean)
	{
		std::shared_ptr<bool> value = std::make_shared<bool>();
		std::cout << "Parsing boolean\n";
		success = parse_bool(*value);
		ptr = value;
	} else if (type_info.type == Text)
	{
		std::shared_ptr<std::string> text = std::make_shared<std::string>();
		success = parse_literal(*text);
		ptr = text;
	} else if (type_info.type == Bytes)
	{
		std::shared_ptr<std::string> vec = std::make_shared<std::string>();
		success = parse_bytes(*vec);
		ptr = vec;
	} else
	{
		printf("Все плохо 5\n");
		exit(-1);
	}

	return success;
}


bool Parser::parse_bool(bool& value)
{
	parse_space_newline_seq();
	std::regex pat("true|false");
	std::string val;
	bool success = parse_pattern(pat, val);

	if (success)
	{
		value = ((val == "true") ? (true) : (false));
	}

	return success;
}

bool Parser::parse_literal(std::string& text)
{
	parse_space_newline_seq();
	std::regex pat("\".*\"");
	std::string val;
	bool success = parse_pattern(pat, val);

	if (success)
	{
		text = std::string(val.begin()+1, val.begin()+(val.size()-1));
	}

	return success;
}


bool Parser::parse_bytes(std::string& text)
{
	parse_space_newline_seq();
	if (parse_hex(text)) {return true;}

	std::regex pat("\".*\"");
	std::string val;
	bool success = parse_pattern(pat, val);

	if (success)
	{
		text = std::string(val.begin()+1, val.begin()+(val.size()-1));
	}

	return success;
}


bool Parser::parse_hex(std::string& text)
{
	std::regex pat("0x[0-9a-fA-F]+");
	std::string val;
	bool success = parse_pattern(pat, val);

	if (success)
	{
		text = convert_hex_to_char(val);
	}

	return success;
}


bool Parser::parse_open()
{
	parse_space_newline_seq();
	std::regex pat("\\(");
	return parse_pattern(pat);
}

bool Parser::parse_close()
{
	parse_space_newline_seq();
	std::regex pat("\\)");
	bool success = parse_pattern(pat);
	return success;
}


void Parser::print_ost()
{
	printf("%s\n", pos_);
}


bool Parser::parse_values(std::string& values)
{
	parse_space_newline_seq();
	std::string val;
	std::regex pat("\\(.*\\)");
	bool success = parse_pattern(pat, val);
	if (success)
	{
		values = std::string(val.begin()+1, val.begin()+(val.size()-1));
	}
	return success;
}


bool Parser::parse_string(std::string str)
{
	parse_space_newline_seq();

	std::regex pat(str);
	std::string val;
	bool success = parse_pattern(pat, val);
	return success;
}


std::vector<std::string> Parser::split_values(const std::string &s, char delim)
{
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
                elems.push_back(item);
        }
        return elems;
}

