#pragma once

#include "utils.hpp"
#include <vector>
#include <map>
#include <memory>


namespace Tables
{

	class Column
	{
	public:
		std::string name;
		TypeInfo info;
		int attributes;
		
		void set_name(std::string value);
		void set_attributes(int flags);
		void set_type(const TypeInfo& type_info);
		void set_default(std::shared_ptr<void> ptr);
		void describe() const;
		std::string get_type_string() const;
		Column get_structure();
		std::shared_ptr<void> get_default();
		bool can_be_added(Column& other);
		void add_values(Column& other);
		void add_value(std::shared_ptr<void>& ptr);
		bool is_unique() const;

		template <typename T>
		T get_value(int index)
		{
			auto vec = std::static_pointer_cast<std::vector<T>>(vec_);
			return (*vec)[index];
		}


	//private:
		std::shared_ptr<void> vec_;
		std::shared_ptr<void> default_value_;
	};

	class Table
	{
	public:
		std::string name;
		int rows;

		Table();

		void set_name(std::string st);
		void add_column(Column column);
		void describe() const;
		Table get_structure();
		Table append(Table& other);
		Column get_column(std::string name);

	//private:
		std::vector<Column> columns_;

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
		    	return ptr_->get_column(name).get_value<T>(index);
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
		bool status;

		QueryResult();
		QueryResult(Table& Table);

		bool is_ok() const;

		Table::TableIterator begin();
		Table::TableIterator end();

	//private:
		std::shared_ptr<Table> table_;
	};


	template <typename T>
	bool is_intersecting(Column& col1, Column& col2)
	{
		std::shared_ptr<std::vector<T>> left = std::static_pointer_cast<std::vector<T>>(col1.vec_); int szl = left->size();
		std::shared_ptr<std::vector<T>> right = std::static_pointer_cast<std::vector<T>>(col2.vec_); int szr = right->size();
		for (int i = 0; i < szl; i++)
		{
			for (int j = 0; j < szr; j++)
			{
				if ((*left)[i] == (*right)[j])
				{
					return true;
				}
			}
		}
		return false;
	}
}

