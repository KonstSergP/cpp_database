#include "../include/table.hpp"
#include "../include/utils.hpp"
#include <iostream>

using namespace Tables;

Table::Table()
{
	rows = 0;
}


void Table::set_name(std::string st)
{
	name = st;
}

void Table::add_column(Column column)
{
	columns_.push_back(column);
}

void Table::describe() const
{
	std::cout << "Table " << name << "\n";
	std::cout << "Columns: ";
	for (const auto& value: columns_)
	{
        std::cout << value.name << " ";
	}
	std::cout << "\n";
	for (const auto& value: columns_)
	{
        value.describe();
	}
}


Table Table::append(Table& other)
{
	if (columns_.size() != other.columns_.size()) {return Table();}

	for (size_t i = 0; i < columns_.size(); i++)
	{
		if (columns_[i].info != other.columns_[i].info)
		{
			return Table();
		}
		if (!columns_[i].can_be_added(other.columns_[i]))
		{
			return Table();
		}
	}
	// good
	for (size_t i = 0; i < columns_.size(); i++)
	{
		columns_[i].add_values(other.columns_[i]);
	}

	std::cout << "Rows: " << rows << " " << other.rows << "\n";
	rows += other.rows;
	std::cout << "Rows: " << rows << " " << other.rows << "\n";

	return other;
}

Table Table::get_structure()
{
	Table table;
	table.set_name(name);
	for (auto& col: columns_)
	{
		table.add_column(col.get_structure());
	}
	return table;
}


void Column::set_attributes(int flags)
{
	attributes = flags;
}


void Column::set_name(std::string value)
{
	name = value;
}


void Column::set_type(const TypeInfo& type_info)
{
	info = type_info;

	switch (type_info.type)
	{
	case Integer:
		vec_ = std::make_shared<std::vector<int32_t>>();
		break;
	case Boolean:
		vec_ = std::make_shared<std::vector<bool>>();
		break;
	case Text:
		vec_ = std::make_shared<std::vector<std::string>>();
		break;
	case Bytes:
		vec_ = std::make_shared<std::vector<std::string>>();
		break;
	default:
		printf("set_type error\n"); exit(-1);
	}
}


void Column::set_default(std::shared_ptr<void> ptr)
{
	default_value_ = ptr;
}

std::string Column::get_type_string() const
{
	std::string result = TypeNames[info.type];
	if (info.length != 1)
	{
		result += "[" + std::to_string(info.length) + "]";
	}
	return result;
}

void Column::describe() const
{
	std::cout << "Column " << name;
	std::cout << " Type: " << get_type_string() << "\n";
}


bool Column::can_be_added(Column& other)
{
	if (!is_unique()) {return true;}

	bool intersect;
	if (info.type == Integer)
	{
		intersect = is_intersecting<int32_t>(*this, other);
	} else if (info.type == Boolean)
	{
		intersect = is_intersecting<bool>(*this, other);
	} else if (info.type == Text)
	{
		intersect = is_intersecting<std::string>(*this, other);
	} else if (info.type == Bytes)
	{
		intersect = is_intersecting<std::string>(*this, other);
	}
	return !intersect;
}

void Column::add_values(Column& other)
{
	if (info.type == Integer)
	{
		auto left = std::static_pointer_cast<std::vector<int32_t>>(vec_);
		auto right = std::static_pointer_cast<std::vector<int32_t>>(other.vec_);
		std::copy(right->begin(), right->end(), std::back_inserter(*left));
	} else if (info.type == Boolean)
	{
		auto left = std::static_pointer_cast<std::vector<bool>>(vec_);
		auto right = std::static_pointer_cast<std::vector<bool>>(other.vec_);
		std::copy(right->begin(), right->end(), std::back_inserter(*left));
		std::cout << "A1: " << vec_ << "\n";
	} else if (info.type == Text)
	{
		auto left = std::static_pointer_cast<std::vector<std::string>>(vec_);
		auto right = std::static_pointer_cast<std::vector<std::string>>(other.vec_);
		std::copy(right->begin(), right->end(), std::back_inserter(*left));
	} else if (info.type == Bytes)
	{
		auto left = std::static_pointer_cast<std::vector<std::string>>(vec_);
		auto right = std::static_pointer_cast<std::vector<std::string>>(other.vec_);
		std::copy(right->begin(), right->end(), std::back_inserter(*left));
	}
}

void Column::add_value(std::shared_ptr<void>& ptr)
{
	if (info.type == Integer)
	{
		auto left = std::static_pointer_cast<std::vector<int32_t>>(vec_);
		auto val = std::static_pointer_cast<int32_t>(ptr);
		left->push_back(*val);
	} else if (info.type == Boolean)
	{	
		std::cout << "add_value func\n";
		auto left = std::static_pointer_cast<std::vector<bool>>(vec_);
		auto val = std::static_pointer_cast<bool>(ptr);
		left->push_back(*val);
	} else if (info.type == Text)
	{
		auto left = std::static_pointer_cast<std::vector<std::string>>(vec_);
		auto val = std::static_pointer_cast<std::string>(ptr);
		left->push_back(*val);
	} else if (info.type == Bytes)
	{
		auto left = std::static_pointer_cast<std::vector<std::string>>(vec_);
		auto val = std::static_pointer_cast<std::string>(ptr);
		left->push_back(*val);
	}
}


std::shared_ptr<void> Column::get_default()
{
	if (!default_value_) {return std::shared_ptr<void>();}
	switch (info.type)
	{
	case Integer:
		return std::make_shared<int32_t>(*std::static_pointer_cast<int32_t>(default_value_));
	case Boolean:
		return std::make_shared<bool>(*std::static_pointer_cast<bool>(default_value_));
	case Text:
		return std::make_shared<std::string>(*std::static_pointer_cast<std::string>(default_value_));
	case Bytes:
		return std::make_shared<std::string>(*std::static_pointer_cast<std::string>(default_value_));
	default:
		printf("get_default error\n"); exit(-1);
	}
}


Column Column::get_structure()
{
	Column column;
	column.set_name(name);
	column.set_attributes(attributes);
	column.set_type(info);
	column.set_default(get_default());

	return column;
}

bool Column::is_unique() const
{
	return attributes & 4;
}

Column Table::get_column(std::string name)
{
	for (auto& column: columns_)
	{
		if (column.name == name)
		{
			return column;
		}
	}
	return Tables::Column();
}


QueryResult::QueryResult(Table& table)
{
	table_ = std::make_shared<Table>(table);
}


bool QueryResult::is_ok() const
{
	return status;
}


QueryResult::QueryResult()
{
	status = true;
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


Table::TableIterator QueryResult::begin()
{
	return table_->begin();
}


Table::TableIterator QueryResult::end()
{
	return table_->end();
}
