#include <iostream>
#include <set>
#include <limits>
#include "../include/database.hpp"
#include "../include/utils.hpp"


//memdb::Database::Database() {}

//memdb::Database::~Database() {}

Tables::QueryResult memdb::Database::execute(std::string query)
{
	parser_.set_query(query);
	parser_.parse_space_newline_seq();

	std::string command;
	if (!parser_.parse_command(command))
	{
		printf("Все плохо 1\n"); exit(-1);
	}
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);

	Tables::QueryResult result;
	if (command == "CREATE")
	{
		if (!parser_.parse_creation(command))
		{
			printf("Все плохо 4\n"); exit(-1);
		}
		std::transform(command.begin(), command.end(), command.begin(), ::toupper);

		std::cout << "execute, command: " << command << "\n";
		result = this->create_table();

	} else if (command == "SELECT")
	{
		result = this->select();
	} else if (command == "UPDATE")
	{
		result = this->update();
	} else if (command == "INSERT")
	{
		std::cout << "INSERT PARSED\n";
		result = this->insert();
	} else if (command == "DELETE")
	{
		result = this->remove();
	} else
	{
		printf("Все плохо 2\n"); exit(-1);
	}

	return result;
}


Tables::Column memdb::Database::make_column()
{
	Tables::Column column;

	int flags;
	parser_.parse_attributes(flags);
	column.set_attributes(flags);

	std::string name;
	parser_.parse_name(name);
	column.set_name(name);

	TypeInfo type_info;
	parser_.parse_type(type_info);
	column.set_type(type_info);

	std::shared_ptr<void> ptr;
	parser_.parse_value(type_info, ptr);
	column.set_default(ptr);
	parser_.parse_comma();

	return column;
}


Tables::QueryResult memdb::Database::create_table()
{
	Tables::Table table;

	std::string name;
	if (!parser_.parse_name(name))
	{
		printf("Все плохо 3\n"); exit(-1);
	}
	std::cout << "table name: " << name << "\n";
	table.set_name(name);


	parser_.parse_open();
	std::cout << "parse_open work\n";
	while (!parser_.parse_close())
	{
		Tables::Column column = make_column();
		table.add_column(column);
	}

	tables_.push_back(table);

	return Tables::QueryResult(table);
}


Tables::QueryResult memdb::Database::insert()
{
	std::string values;
	parser_.parse_values(values);
	parser_.parse_string("to");

	std::string name;
	parser_.parse_name(name);
	std::cout << "Parsed name: " << name << "\n";
	std::cout << "Parsed values: " << values << "\n";

	Tables::Table table = get_table(name).get_structure();

	auto vals = parser_.split_values(values, ',');

	if (vals[0].find("=") != ULONG_MAX)
	{
		std::set<std::string> names;
		for (const auto& val: table.columns_)
		{
			std::cout << val.name << "\n";
			names.insert(val.name);
		}
		std::cout << "\n";
		for (auto& arg: vals)
		{
			parser_.set_query(arg);
			std::shared_ptr<void> ptr;

			std::string col_name;
			parser_.parse_name(col_name);
			std::cout << col_name << "\n";
			parser_.parse_value(table.get_column(col_name).info, ptr);
			std::cout << "parsed_value\n";

			table.get_column(col_name).add_value(ptr);
			std::cout << "value added\n";
			names.erase(col_name);
			std::cout << "one done\n";
		}
		for (auto& arg: names)
		{
			auto ptr = table.get_column(arg).get_default();
			table.get_column(arg).add_value(ptr);
		}
	} else
	{
		int cnt = 0;
		for (size_t i = 0; i < table.columns_.size(); i++)
		{
			parser_.set_query("="+vals[cnt]);
			std::shared_ptr<void> ptr; 
			std::cout << vals[cnt] << " " << table.columns_[i].info.type << "\n";
			if (!parser_.parse_value(table.columns_[i].info, ptr))
			{
				ptr = table.columns_[i].get_default();
			} else {cnt++;}
			table.columns_[i].add_value(ptr);
		}
	}
	table.rows = 1;

	get_table(name).append(table);

	return Tables::QueryResult(table);
}

Tables::Table& memdb::Database::get_table(std::string name)
{
	for (auto& table: tables_)
	{
		if (table.name == name)
		{
			return table;
		}
	}
	return tables_[0];
}



Tables::QueryResult memdb::Database::update()
{
	return Tables::QueryResult();
}


Tables::QueryResult memdb::Database::remove()
{
	return Tables::QueryResult();
}


Tables::QueryResult memdb::Database::select()
{
	return Tables::QueryResult();
}

void memdb::Database::describe()
{
	std::cout << "Tables: ";
	for (const auto& value: tables_)
	{
        std::cout << value.name << " ";
	}
	std::cout << "\n";
	for (const auto& value: tables_)
	{
        value.describe();
	}
}

