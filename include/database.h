#pragma once

#include "parser.h"
#include "table.h"
#include "column.h"
#include "parser.h"
#include "utils.h"
#include "ErrorHandler.h"
#include "evaluator.h"
#include <map>
#include <iostream>

namespace memdb
{
	class Database
	{
	public:

		Tables::QueryResult execute(std::string query);
		void describe();
		void load_from_file(std::ifstream&& in);
		void save_to_file(std::ofstream&& out);

	//private:
		
		std::map<std::string, Tables::Table> tables_;
		Parsing::Parser parser_;

		Tables::Table& get(std::string name);
		Tables::QueryResult create_table();
		Tables::QueryResult insert();
		Tables::QueryResult update();
		Tables::QueryResult remove();
		Tables::QueryResult select();
		std::shared_ptr<Columns::Column> make_column();
		std::shared_ptr<Evaluator> calculate(Tables::Table& table);
	};
}
