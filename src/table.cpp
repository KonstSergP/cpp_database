#include "../include/table.h"
#include "../include/column.h"
#include "../include/utils.h"
#include <iostream>
#include <variant>
#include <fstream>
#include "../include/ErrorHandler.h"

using namespace Tables;
using namespace Columns;

Table::Table(): rows(0)
{}


void Table::set_name(std::string st)
{
	name = st;
}

void Table::add_column(std::shared_ptr<Column> col)
{
	columns_.push_back(col);
}

void Table::describe() const
{
	std::cout << "Table " << name << "\n";
	std::cout << "Columns: ";
	for (const auto& value: columns_)
	{
        std::cout << value->name << " ";
	}
	std::cout << "\n";
	for (const auto& value: columns_)
	{
        value->describe();
	}
}


void Table::append(Table& other)
{
	if (columns_.size() != other.columns_.size())
	{
		throw TableException(TE::DIFF_COL_NUM);
	}

	for (size_t i = 0; i < columns_.size(); i++)
	{
		if (columns_[i]->info != other.columns_[i]->info)
		{
			throw TableException(TE::DIFF_COL_TYPE);
		}
		if (!columns_[i]->can_be_added(other.columns_[i]))
		{
			throw TableException(TE::NO_UNIQUE);
		}
	}

	for (size_t i = 0; i < columns_.size(); i++)
	{
		columns_[i]->add_values(other.columns_[i]);
		if (columns_[i]->is_autoinc())
		{
			columns_[i]->change_default(other.columns_[i]);
		}
	}

	rows += other.rows;
}

Table Table::get_structure()
{
	Table table;
	table.set_name(name);
	for (auto& col: columns_)
	{
		table.add_column(col->get_structure());
	}
	return table;
}


std::shared_ptr<Column> Table::get(std::string name)
{
	for (auto& column: columns_)
	{
		if (column->name == name)
		{
			return column;
		}
	}
	throw TableException(TE::CNEXIST);
}


bool QueryResult::is_ok() const
{
	return status_ok;
}


QueryResult::QueryResult(std::shared_ptr<Table> table)
{
	status_ok = true;
	var = table;
}


QueryResult::QueryResult()
{
	status_ok = false;
}

QueryResult::QueryResult(TableException& e)
{
	status_ok = false;
	var = e;
}


void QueryResult::what()
{
	std::cout << std::get<TableException>(var).what();
}


Table::TableIterator QueryResult::begin()
{
	return std::get<std::shared_ptr<Table>>(var)->begin();
}


Table::TableIterator QueryResult::end()
{
	return std::get<std::shared_ptr<Table>>(var)->end();
}


Table::TableIterator::TableIterator(int ind, Table* ptr): index(ind), ptr_(ptr)
{}


Table::TableIterator& Table::TableIterator::operator++()
{
	index++;
	return *this;
}


Table::TableIterator Table::TableIterator::operator++(int)
{ 
	TableIterator tmp = *this;
	index++;
	return tmp;

}


bool Table::TableIterator::operator==(const Table::TableIterator& other)
{
	return (ptr_ == other.ptr_) && (index == other.index);
}


bool Table::TableIterator::operator!=(const Table::TableIterator& other)
{
	return (ptr_ != other.ptr_) || (index != other.index);
}

Table::TableIterator& Table::TableIterator::operator*()
{
	return *this;
}


Table::TableIterator Table::begin()
{
	return Table::TableIterator(0, this);
}


Table::TableIterator Table::end()
{
	return Table::TableIterator(rows, this);
}


Table Table::load_from_file(std::ifstream& in, std::string& name)
{
	auto res = Table();

	size_t sz; 		in.read(reinterpret_cast<char*>(&sz), sizeof(sz));
	char nm[sz+1];	in.read(nm, sz); nm[sz]=0; name = std::string(nm); res.set_name(name);
					in.read(reinterpret_cast<char*>(&sz), sizeof(sz));
	int rws; 		in.read(reinterpret_cast<char*>(&rws), sizeof(rws)); res.rows = rws;

	for (size_t i = 0; i < sz; i++) {
		res.add_column(Columns::Column::load_from_file(in, rws));
	}

	return res;
}


void Table::save_to_file(std::ofstream& out)
{
	auto sz = name.size();
	out.write(reinterpret_cast<char*>(&sz), sizeof(sz));
	out.write(name.c_str(), sz);
	sz = columns_.size();
	out.write(reinterpret_cast<char*>(&sz), sizeof(sz));

	out.write(reinterpret_cast<char*>(&rows), sizeof(rows));

	for (auto& v: columns_) {
		v->save_to_file(out);
	}
}
