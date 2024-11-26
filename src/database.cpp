#include <iostream>
#include <set>
#include <stack>
#include <limits>
#include <memory>
#include <fstream>
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


std::shared_ptr<Columns::Column> memdb::Database::make_column()
{
	bool key = false, unique = false, autoinc = false;
	std::string name;
	TypeInfo type_info;

	if (!parser_.parse_attributes(key, unique, autoinc)) {
		throw TableException(TE::PARSE_ATTR);
	}
	if (!parser_.parse_name(name)) {
		throw TableException(TE::PARSE_CNAME);
	}
	if (!parser_.parse_type(type_info)) {
		throw TableException(TE::PARSE_TYPE);
	}

	auto column = Columns::CreateColumn(type_info);
	column->set_name(name);
	column->set_attributes(key, unique, autoinc);

	std::shared_ptr<void> ptr;
	if (parser_.parse_equal_sign())
	{
		if(!parser_.parse_value(false, type_info, ptr)) {
			throw TableException(TE::PARSE_VAL);
		}
	}
	column->set_default(ptr);
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
		auto column = make_column();
		table.add_column(column);
	}

	tables_[name] = table;

	return Tables::QueryResult(std::make_shared<Tables::Table>(table));
}


Tables::QueryResult memdb::Database::insert()
{
	std::string values, name;
	if (!parser_.parse_values(values)) {
		throw TableException(TE::PARSE_VALS);
	}
	parser_.parse_string("to");

	if (!parser_.parse_name(name)) {
		throw TableException(TE::PARSE_TNAME);
	}

	Tables::Table table = get(name).get_structure();

	auto vals = parser_.split_values(values, ',');

	if (vals[0].find("=") != ULONG_MAX)
	{
		std::set<std::string> names;
		for (const auto& val: table.columns_) {
			names.insert(val->name);
		}

		for (auto& arg: vals)
		{
			parser_.set_query(arg);
			std::shared_ptr<void> ptr;

			std::string col_name;
			if (!parser_.parse_name(col_name)) {
				throw TableException(TE::PARSE_CNAME);
			}
			if (!parser_.parse_value(true, table.get(col_name)->info, ptr)) {
				throw TableException(TE::PARSE_VAL);
			}

			table.get(col_name)->add_value(ptr);
			names.erase(col_name);
		}
		for (auto& arg: names)
		{
			auto col = table.get(arg);
			auto ptr = col->get_default();
			if (!ptr) {
				throw TableException(TE::NO_DEFAULT);
			}
			col->add_value(ptr);
			if (col->is_autoinc() && col->info.type == Integer)
			{
				(*std::static_pointer_cast<int>(ptr))++;
				col->set_default(ptr);
			}
		}
	} else
	{
		if (table.columns_.size() != vals.size()) {
			throw TableException(TE::WCNT_VALS);
		}
		for (size_t i = 0; i < table.columns_.size(); i++)
		{
			parser_.set_query(vals[i]);
			std::shared_ptr<void> ptr; 
			if (!parser_.parse_value(false, table.columns_[i]->info, ptr))
			{
				if (parser_.end_of_query())
				{
					ptr = table.columns_[i]->get_default();
					if (!ptr) {
						throw TableException(TE::NO_DEFAULT);
					}
					if (table.columns_[i]->is_autoinc() && table.columns_[i]->info.type == Integer)
					{
						(*std::static_pointer_cast<int>(ptr))++;
						table.columns_[i]->set_default(ptr);
					}
				} else {
					throw TableException(TE::PARSE_VAL);
				}
			}
			table.columns_[i]->add_value(ptr);
		}
	}
	table.rows = 1;
	get(name).append(table);

	return Tables::QueryResult(std::make_shared<Tables::Table>(table));
}


Tables::Table& memdb::Database::get(std::string name)
{
	auto res = tables_.find(name);
	if (res != tables_.end())
	{
		return res->second;
	}
	throw TableException(TE::TNEXIST);
}


Tables::QueryResult memdb::Database::select()
{
	std::shared_ptr<Tables::Table> res = std::make_shared<Tables::Table>();
	res->set_name("select result");

	std::vector<std::string> calcs;
	if (!parser_.parse_before_from(calcs)) {
		throw TableException(TE::WRONG_EXPRS);
	}

	std::string name;
	if (!parser_.parse_name(name)) {
		throw TableException(TE::PARSE_TNAME);
	}

	parser_.parse_where();
	auto a = std::static_pointer_cast<std::vector<bool>>(calculate(get(name))->evaluate(nullptr));
	res->rows = std::count_if(a->begin(), a->end(), [](bool b){return b;});

	int cnt = 0;
	for (auto s: calcs)
	{
		cnt++;
		parser_.set_query(s);
		auto calc = calculate(get(name));
		auto b = calc->evaluate(a);
		auto new_col = Columns::CreateColumn(calc->value_type);
		new_col->set_name(std::to_string(cnt));
		new_col->set_values(b);
		res->add_column(new_col);
	}

	return Tables::QueryResult(res);
}


Tables::QueryResult memdb::Database::update()
{
	std::shared_ptr<Tables::Table> res = std::make_shared<Tables::Table>();
	res->set_name("update result");

	std::string name;
	if (!parser_.parse_name(name)) {
		throw TableException(TE::PARSE_TNAME);
	}
	auto& tab = get(name);

	if(!parser_.parse_string("set|SET")) {
		throw TableException(TE::NO_KEYWORD);
	}
	
	std::vector<std::string> calcs; bool where_found;
	parser_.parse_before_where(calcs, where_found);

	std::shared_ptr<std::vector<bool>> a = nullptr;
	if (where_found) {
		a = std::static_pointer_cast<std::vector<bool>>(calculate(get(name))->evaluate(nullptr));
		res->rows = std::count_if(a->begin(), a->end(), [](bool b){return b;});
	} else {
		res->rows = tab.rows;
	}

	for (auto s: calcs)
	{
		parser_.set_query(s);
		if (!parser_.parse_name(name)) {
			throw TableException(TE::PARSE_CNAME);
		}
		if (!parser_.parse_equal_sign()) {
			throw TableException(TE::NO_OPERATOR);
		}

		auto calc = calculate(tab);
		auto b = calc->evaluate(a);
		tab.get(name)->replace(b, a);
		auto new_col = Columns::CreateColumn(calc->value_type);
		new_col->set_name(name);
		new_col->set_values(b);
		res->add_column(new_col);
	}

	return Tables::QueryResult(res);
}


Tables::QueryResult memdb::Database::remove()
{
	std::shared_ptr<Tables::Table> res = std::make_shared<Tables::Table>();
	res->set_name("delete result");

	std::string name;
	if (!parser_.parse_name(name)) {
		throw TableException(TE::PARSE_TNAME);
	}
	auto& tab = get(name);

	parser_.parse_where();
	auto a = std::static_pointer_cast<std::vector<bool>>(calculate(get(name))->evaluate(nullptr));
	res->rows = std::count_if(a->begin(), a->end(), [](bool b){return b;});

	for (auto& col: tab.columns_)
	{
		auto b = col->extract(a);
		auto new_col = col->get_structure();
		new_col->set_values(b);
		res->add_column(new_col);
	}
	tab.rows -= res->rows;

	return Tables::QueryResult(res);
}


void memdb::Database::describe()
{
	std::cout << "Tables: ";
	for (const auto& value: tables_)
	{
        std::cout << value.second.name << " ";
	}
	std::cout << "\n";
	for (const auto& value: tables_)
	{
        value.second.describe();
	}
}


std::shared_ptr<Evaluator> memdb::Database::calculate(Tables::Table& table)
{
	std::vector<std::shared_ptr<Evaluator>> vec;
	std::stack<std::shared_ptr<Evaluator>> stack; 
	std::string token;
	bool len_opened = false;

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
		else if (token == "|")
		{
			if (!len_opened)
			{
				len_opened = true;
				stack.push(std::make_shared<Evaluator>(nullptr, NODE_TYPE::ABS));
			}
			else
			{
				len_opened = false;
				while (stack.top()->node_type != NODE_TYPE::ABS)
				{
					vec.push_back(stack.top());
					stack.pop();
				}
				vec.push_back(stack.top());	
				stack.pop();
			}
		}
		else if (TOKEN_TO_OPERATOR.find(token) != TOKEN_TO_OPERATOR.end())
		{
			if (stack.empty())
			{
				stack.push(std::make_shared<Evaluator>(nullptr, TOKEN_TO_OPERATOR[token]));
			}
			else if (prior(TOKEN_TO_OPERATOR[token]) < prior(stack.top()->node_type))
			{
				stack.push(std::make_shared<Evaluator>(nullptr, TOKEN_TO_OPERATOR[token]));
			} else
			{
				while (!stack.empty() && prior(TOKEN_TO_OPERATOR[token]) >= prior(stack.top()->node_type))
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
				if (col->name == token)
				{
					vec.push_back(std::make_shared<Evaluator>(col->get_values(), NODE_TYPE::COLUMN, col->info.type));
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

	for (auto& a: vec)
	{
		if (a->node_type == NODE_TYPE::VALUE || a->node_type == NODE_TYPE::COLUMN) {
			stack.push(a);
		}
		else if (a->node_type == NODE_TYPE::NOT || a->node_type == NODE_TYPE::ABS)
		{
			a->left = stack.top();
			stack.pop();
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

	std::shared_ptr<EvaluatorHead> head = std::make_shared<EvaluatorHead>();
	head->set_rows(table.rows);
	head->left = stack.top();
	return head;
}


void memdb::Database::load_from_file(std::ifstream&& in)
{
	tables_.clear();
	size_t sz; in.read(reinterpret_cast<char*>(&sz), sizeof(sz));
	std::string name;

	for (size_t i = 0; i < sz; i++)
	{
		auto tb = Tables::Table::load_from_file(in, name);
		tables_[name] = tb;
	}
}


void memdb::Database::save_to_file(std::ofstream&& out)
{
	auto sz = tables_.size();
	out.write(reinterpret_cast<char*>(&sz), sizeof(sz));

	for (auto& [k, v]: tables_)
	{
		v.save_to_file(out);
	}
}
