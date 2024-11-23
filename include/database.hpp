#pragma once

#include "parser.hpp"
#include "table.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include "ErrorHandler.hpp"
#include <map>

namespace memdb
{
	class Database
	{
	public:

		Tables::QueryResult execute(std::string query);
		void describe();
		Tables::Table& get(std::string name);

	//private:
		
		std::vector<Tables::Table> tables_;
		Parsing::Parser parser_; 

		Tables::QueryResult create_table();
//		Tables::QueryResult create_index();
		Tables::QueryResult insert();
		Tables::QueryResult update();
		Tables::QueryResult remove();
		Tables::QueryResult select();
		Tables::Column make_column();
		std::shared_ptr<void> calculate(Tables::Table& table);
	};
}
