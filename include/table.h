#pragma once

#include "utils.h"
#include "column.h"
#include "ErrorHandler.h"
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <fstream>


namespace Tables
{
	using namespace Columns;
	class Table
	{
	public:
		std::string name;
		int rows;

		Table();

		void set_name(std::string st);
		void add_column(std::shared_ptr<Column> col);
		void describe() const;
		Table get_structure();
		void append(Table& other);
		std::shared_ptr<Column> get(std::string name);
		void save_to_file(std::ofstream& out);
		static Table load_from_file(std::ifstream& in, std::string& name);

	//private:
		std::vector<std::shared_ptr<Column>> columns_;

		class TableIterator
		{
		public:
			TableIterator(int ind, Tables::Table* ptr);

		    TableIterator& operator++(); 
		    TableIterator operator++(int);
		    TableIterator& operator*();

		    bool operator==(const TableIterator& other);
		    bool operator!=(const TableIterator& other);

		    template <typename T>
		    T get(std::string name)
		    {
		    	return ptr_->get(name)->get<T>(index);
		    }

		    int index;
		    Tables::Table* ptr_;
		};
		TableIterator begin();
		TableIterator end();
	};

	class QueryResult
	{
	public:

		QueryResult();
		QueryResult(std::shared_ptr<Table> Table);
		QueryResult(TableException& e);

		bool is_ok() const;

		Table::TableIterator begin();
		Table::TableIterator end();
		void what();

	//private:
		bool status_ok;
		std::variant<std::shared_ptr<Table>, TableException> var;
	};
}

