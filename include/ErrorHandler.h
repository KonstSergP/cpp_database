#pragma once
#include <map>
#include <string>


enum class TE
{
    NO_COMMAND,
    NOCR_OBJ,
    PARSE_ATTR,
    PARSE_CNAME,
    PARSE_TYPE,
    PARSE_VAL,
    PARSE_TNAME,
    EXP_OPEN,
    PARSE_VALS,
    NO_DEFAULT,
    WCNT_VALS,
    NO_UNIQUE,
    DIFF_COL_NUM,
    DIFF_COL_TYPE,
    UNKNOWN_TYPE,
    CNEXIST,
    TNEXIST,
    WRONG_EXPRS,
    NO_KEYWORD,
    UNEXP_QUERY,
    NO_OPERATOR
};



extern std::map<TE, std::string> TE_messages;

class TableException : public std::exception
{
public:
    TableException(TE code);
    const char* what() const noexcept override;

	std::string what_message;
};