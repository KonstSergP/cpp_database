#include "../include/table.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <variant>
#include "../include/ErrorHandler.hpp"

using namespace Tables;

Table::Table(): rows(0)
{}


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


void Table::append(Table& other)
{
	if (columns_.size() != other.columns_.size())
	{
		throw TableException(TE::DIFF_COL_NUM);
	}

	for (size_t i = 0; i < columns_.size(); i++)
	{
		if (columns_[i].info != other.columns_[i].info)
		{
			throw TableException(TE::DIFF_COL_TYPE);
		}
		if (!columns_[i].can_be_added(other.columns_[i]))
		{
			throw TableException(TE::NO_UNIQUE);
		}
	}

	for (size_t i = 0; i < columns_.size(); i++)
	{
		columns_[i].add_values(other.columns_[i]);
		if (columns_[i].is_autoinc())
		{
			columns_[i].change_default(other.columns_[i]);
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
		throw TableException(TE::UNKNOWN_TYPE);
	}
}


void Column::set_default(std::shared_ptr<void> ptr)
{
	default_value_ = ptr;
}


#define VALT(x, t) std::static_pointer_cast<t>(x)

void Column::change_default(Column& other)
{
	switch (info.type)
	{
	case Integer:
		default_value_ = std::make_shared<int32_t>(*VALT(other.default_value_, int32_t));
		break;
	case Boolean:
		default_value_ = std::make_shared<bool>(*VALT(other.default_value_, bool));
		break;
	case Text:
		default_value_ = std::make_shared<std::string>(*VALT(other.default_value_, std::string));
		break;
	case Bytes:
		default_value_ = std::make_shared<std::string>(*VALT(other.default_value_, std::string));
		break;
	default:
		throw TableException(TE::UNKNOWN_TYPE);
	}
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
	} else
	{
		throw TableException(TE::UNKNOWN_TYPE);
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
	} else
	{
		throw TableException(TE::UNKNOWN_TYPE);
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
	} else
	{
		throw TableException(TE::UNKNOWN_TYPE);
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
		throw TableException(TE::UNKNOWN_TYPE);
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


bool Column::is_autoinc() const
{
	return attributes & 2;
}


bool Column::is_key() const
{
	return attributes == 5;
}


Column& Table::get(std::string name)
{
	for (auto& column: columns_)
	{
		if (column.name == name)
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


QueryResult::QueryResult(Table& table)
{
	status_ok = true;
	var = std::make_shared<Table>(table);
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
