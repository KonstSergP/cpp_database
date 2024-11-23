#include <iostream>
#include <stdlib.h>
#include <regex>
#include <string>
#include <cstdint>

#include "../include/parser.h"
#include "../include/utils.h"

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
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
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
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
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
				return false;
			}
			parse_comma();
		}
		if (!parse_close_fig()) {return false;}
	}
	else
	{
		flags = 0;
	}
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
	if (!success) {return false;}

	if (type == "int32")
	{
		type_info.type = Integer;
		type_info.length = 1;
		return true;
	} else if (type == "bool")
	{
		type_info.type = Boolean;
		type_info.length = 1;
		return true;
	}
	int len;
	if (!parse_array_len(len))
	{
		return false;
	}

	if (type == "string")
	{
		type_info.type = Text;
		type_info.length = len;
	} else if (type == "bytes")
	{
		type_info.type = Bytes;
		type_info.length = len;
	} else
	{
		return false;
	}

	return true;
}


bool Parser::parse_array_len(int& len)
{
	parse_space_newline_seq();
	bool success = true;
	success &= parse_pattern(std::regex("\\["));
	success &= parse_number(len);
	success &= parse_pattern(std::regex("\\]"));

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


bool Parser::parse_value(bool eq, TypeInfo type_info, std::shared_ptr<void>& ptr)
{
	bool success;
	parse_space_newline_seq();
	if (eq)
	{
		if (!parse_equal_sign()) {return false;}
	}

	if (type_info.type == Integer)
	{
		std::shared_ptr<int> number = std::make_shared<int>();
		success = parse_number(*number);
		ptr = number;
	} else if (type_info.type == Boolean)
	{
		std::shared_ptr<bool> value = std::make_shared<bool>();
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
		success = false;
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
	bool success = parse_pattern(pat, text);
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


bool Parser::end_of_query()
{
	return pos_ == end_;
}


bool Parser::parse_token(std::string& token)
{
	parse_space_newline_seq();
	std::regex pat("\\(|\\)|0x[0-9a-fA-F]+|0|(\\+|\\-)?[1-9][0-9]*|\".*\"|[\\+\\-\\*/%><=]|\\&\\&|\\^\\^|>=|<=|!=|[a-zA-Z_]+");
	bool success = parse_pattern(pat, token);
	return success;
}


bool Parser::parse_token_value(TypeInfo type_info, std::shared_ptr<void>& ptr, std::string token)
{
	const char* old_ptr = pos_, *old_end = end_;
	pos_ = token.c_str(); end_ = token.c_str()+token.size();

	bool success = parse_value(false, type_info, ptr);
	pos_ = old_ptr; end_ = old_end;
	return success;
}


bool Parser::is_end()
{
	return pos_ == end_;
}
