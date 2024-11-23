#include <iostream>
#include <set>
#include <stack>
#include <limits>
#include <memory>
#include "../include/database.h"
#include "../include/utils.h"
#include "../include/ErrorHandler.h"
#include "../include/evaluator.h"



Tables::QueryResult memdb::Database::execute(std::string query)
{
	Tables::QueryResult result;

	try
	{
		parser_.set_query(query);
		parser_.parse_space_newline_seq();

		std::string command;
		if (!parser_.parse_command(command))
		{
			throw TableException(TE::NO_COMMAND);
		}

		if (command == "CREATE")
		{
			if (!parser_.parse_creation(command))
			{
				throw TableException(TE::NOCR_OBJ);
			}

			result = create_table();

		} else if (command == "SELECT")
		{
			result = select();
		} else if (command == "UPDATE")
		{
			result = update();
		} else if (command == "INSERT")
		{
			result = insert();
		} else if (command == "DELETE")
		{
			result = remove();
		}
	} catch (TableException& e)
	{
		return Tables::QueryResult(e);
	}

	return result;
}


Tables::Column memdb::Database::make_column()
{
	Tables::Column column;

	int flags;
	if (!parser_.parse_attributes(flags))
	{
		throw TableException(TE::PARSE_ATTR);
	}
	column.set_attributes(flags);

	std::string name;
	if (!parser_.parse_name(name))
	{
		throw TableException(TE::PARSE_CNAME);
	}
	column.set_name(name);

	TypeInfo type_info;
	if (!parser_.parse_type(type_info))
	{
		throw TableException(TE::PARSE_TYPE);
	}
	column.set_type(type_info);

	std::shared_ptr<void> ptr;
	if (parser_.parse_equal_sign())
	{
		if(!parser_.parse_value(false, type_info, ptr))
		{
			throw TableException(TE::PARSE_VAL);
		}
	}
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
		throw TableException(TE::PARSE_TNAME);
	}
	table.set_name(name);

	if (!parser_.parse_open())
	{
		throw TableException(TE::EXP_OPEN);
	}
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
	std::string values, name;
	if (!parser_.parse_values(values))
	{
		throw TableException(TE::PARSE_VALS);
	}
	parser_.parse_string("to");

	if (!parser_.parse_name(name))
	{
		throw TableException(TE::PARSE_TNAME);
	}

	Tables::Table table = get(name).get_structure();

	auto vals = parser_.split_values(values, ',');

	if (vals[0].find("=") != ULONG_MAX)
	{
		std::set<std::string> names;
		for (const auto& val: table.columns_)
		{
			names.insert(val.name);
		}

		for (auto& arg: vals)
		{
			parser_.set_query(arg);
			std::shared_ptr<void> ptr;

			std::string col_name;
			if (!parser_.parse_name(col_name))
			{
				throw TableException(TE::PARSE_CNAME);
			}
			if (!parser_.parse_value(true, table.get(col_name).info, ptr))
			{
				throw TableException(TE::PARSE_VAL);
			}

			table.get(col_name).add_value(ptr);
			names.erase(col_name);
		}
		for (auto& arg: names)
		{
			auto& col = table.get(arg);
			auto ptr = col.get_default();
			if (!ptr)
			{
				throw TableException(TE::NO_DEFAULT);
			}
			col.add_value(ptr);
			if (col.is_autoinc() && col.info.type == Integer) {(*std::static_pointer_cast<int>(col.default_value_))++;}
		}
	} else
	{
		if (table.columns_.size() != vals.size())
		{
			throw TableException(TE::WCNT_VALS);
		}
		for (size_t i = 0; i < table.columns_.size(); i++)
		{
			parser_.set_query(vals[i]);
			std::shared_ptr<void> ptr; 
			if (!parser_.parse_value(false, table.columns_[i].info, ptr))
			{
				if (parser_.end_of_query())
				{
					ptr = table.columns_[i].get_default();
					if (!ptr)
					{
						throw TableException(TE::NO_DEFAULT);
					}
				} else
				{
					throw TableException(TE::PARSE_VAL);
				}
			}
			table.columns_[i].add_value(ptr);
			if (table.columns_[i].is_autoinc() && table.columns_[i].info.type == Integer) {(*std::static_pointer_cast<int>(table.columns_[i].default_value_))++;}
		}
	}
	table.rows = 1;
	get(name).append(table);

	return Tables::QueryResult(table);
}


Tables::Table& memdb::Database::get(std::string name)
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


Tables::QueryResult memdb::Database::select()
{
	calculate(tables_[0]);
	return Tables::QueryResult();
}


Tables::QueryResult memdb::Database::update()
{
	return Tables::QueryResult();
}


Tables::QueryResult memdb::Database::remove()
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


std::shared_ptr<void> memdb::Database::calculate(Tables::Table& table)
{	
	///////
	table.get("id");
	///////

	std::vector<std::shared_ptr<Evaluator>> vec;
	std::stack<std::shared_ptr<Evaluator>> stack; 
	std::string token;

	while (parser_.parse_token(token))
	{
		if (token == "(")
		{
			stack.push(std::make_shared<Evaluator>(nullptr, NODE_TYPE::OPEN_BRACE));
		}
		else if (token == ")")
		{	
			while (stack.top()->node_type != NODE_TYPE::OPEN_BRACE)
			{
				vec.push_back(stack.top());
				stack.pop();
			}
			stack.pop();
		}
		else if (TOKEN_TO_OPERATOR.find(token) != TOKEN_TO_OPERATOR.end())
		{
			if (stack.empty())
			{
				stack.push(std::make_shared<Evaluator>(nullptr, TOKEN_TO_OPERATOR[token]));
			}
			else if (prior(TOKEN_TO_OPERATOR[token]) > prior(stack.top()->node_type))
			{
				stack.push(std::make_shared<Evaluator>(nullptr, TOKEN_TO_OPERATOR[token]));
			} else
			{
				while (!stack.empty() && prior(TOKEN_TO_OPERATOR[token]) <= prior(stack.top()->node_type))
				{
					vec.push_back(stack.top());
					stack.pop();
				}
				stack.push(std::make_shared<Evaluator>(nullptr, TOKEN_TO_OPERATOR[token]));
			}
		}
		else
		{
			bool fnd = false;
			for (auto& col: table.columns_)
			{
				if (col.name == token)
				{
					vec.push_back(std::make_shared<Evaluator>(col.vec_, NODE_TYPE::COLUMN, col.info.type));
					fnd = true;
					break;
				}
			}
			if (fnd) {continue;}

			std::shared_ptr<void> val;
			Types tp;
			if (parser_.parse_token_value(Bytes, val, token))        {tp = Bytes;}
			else if (parser_.parse_token_value(Integer, val, token)) {tp = Integer;}
			else if (parser_.parse_token_value(Boolean, val, token)) {tp = Boolean;}
			else if (parser_.parse_token_value(Text, val, token))    {tp = Text;}
			if (!val)
			{
				throw TableException(TE::PARSE_VAL);
			}
			vec.push_back(std::make_shared<Evaluator>(val, NODE_TYPE::VALUE, tp));	
		}

	}

	while (!stack.empty())
	{
		vec.push_back(stack.top());
		stack.pop();
	}

	std::cout << "Size: " << vec.size() << "\n";
	for (auto& a: vec)
	{
		a->print();
	}std::cout << "n\n";


	for (auto& a: vec)
	{
		if (a->node_type == NODE_TYPE::VALUE || a->node_type == NODE_TYPE::COLUMN)
		{
			stack.push(a);
		}
		else
		{
			a->right = stack.top();
			stack.pop();
			a->left = stack.top();
			stack.pop();
			stack.push(a);
		}
	}

	return stack.top()->evaluate();
}



	
