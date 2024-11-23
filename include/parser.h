#pragma once

#include "utils.h"
#include <string>
#include <regex>
#include <memory>

namespace Parsing
{
	class Parser
	{
	public:

		Parser() = default;

		Parser(const Parser& parser) = delete;
		Parser(Parser&& parser) = default;

		Parser& operator=(const Parser& parser) = delete;
		Parser& operator=(Parser&& parser) = default;

		void set_query(std::string);
		bool parse_command(std::string& command);
		bool parse_name(std::string& name);
		bool parse_creation(std::string& name);
		bool parse_attributes(int& flags);
		bool parse_comma();
		bool parse_type(TypeInfo& type_info);
		bool parse_colon();
		bool parse_number(int& number);
		bool parse_value(bool eq, TypeInfo type_info, std::shared_ptr<void>& ptr);
		bool parse_values(std::string& values);
		bool parse_equal_sign();
		bool parse_space_newline_seq();
		bool parse_open();
		bool parse_close();
		void print_ost();
		bool parse_string(std::string str);
		std::vector<std::string> split_values(const std::string &s, char delim);
		bool parse_literal(std::string& text);
		bool end_of_query();
		bool parse_token(std::string& token);
		bool parse_token_value(TypeInfo type_info, std::shared_ptr<void>& ptr, std::string token);
		bool is_end();

	//private:

		const char* pos_;
		const char* end_;
		std::string query_;

		bool parse_pattern(std::regex regexp);
		bool parse_pattern(std::regex regexp, std::string& ret);

		bool parse_attr(std::string& attribute);
		bool parse_open_fig();
		bool parse_close_fig();
		bool parse_bool(bool& value);
		bool parse_bytes(std::string& text);
		bool parse_hex(std::string& text);
		bool parse_array_len(int& len);

	}; // class Parser
}
