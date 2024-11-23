#include <iostream>
#include "../include/utils.hpp"
#include "../include/evaluator.hpp"
#include <map>


std::map<std::string, NODE_TYPE> TOKEN_TO_OPERATOR = 
{
	{"+", NODE_TYPE::ADDITION},
	{"-", NODE_TYPE::SUBSTRACTION},
	{"*", NODE_TYPE::MULTYPLICATION},
	{"/", NODE_TYPE::DIVISION},
	{"%", NODE_TYPE::MOD},
	{"<", NODE_TYPE::LESS},
	{"=", NODE_TYPE::EQUAL},
	{">", NODE_TYPE::GREATER},
	{"<=", NODE_TYPE::LE},
	{">=", NODE_TYPE::GE},
	{"!=", NODE_TYPE::N_EQUAL},
	{"&&", NODE_TYPE::AND},
	{"||", NODE_TYPE::OR},
	{"!", NODE_TYPE::NOT},
	{"^^", NODE_TYPE::XOR},
	{"|", NODE_TYPE::ABS},
	{"(", NODE_TYPE::OPEN_BRACE}
};


int prior(NODE_TYPE type)
{
	switch(type)
	{
    case NODE_TYPE::MULTYPLICATION:
    case NODE_TYPE::DIVISION:
        return 4;
    case NODE_TYPE::SUBSTRACTION:
    case NODE_TYPE::ADDITION:
        return 3;
    case NODE_TYPE::EQUAL:
    	return 2;
    case NODE_TYPE::OPEN_BRACE:
        return 1;
    default:
    	return 0;
	}
}

std::shared_ptr<std::vector<int>> arithm_op(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE n_type);
std::shared_ptr<std::vector<bool>> equal(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, Types type);

Evaluator::Evaluator(std::shared_ptr<void> ptr, NODE_TYPE type): node_type(type), ptr_(ptr)
{}


Evaluator::Evaluator(std::shared_ptr<void> ptr, NODE_TYPE type, Types val_type): node_type(type), value_type(val_type), ptr_(ptr)
{}


std::shared_ptr<void> Evaluator::evaluate()
{
	if (node_type == NODE_TYPE::VALUE || node_type == NODE_TYPE::COLUMN)
	{
		return ptr_;
	}
	auto x = left->evaluate(); 	Types lt = left->value_type;
	auto y = right->evaluate(); //Types rt = right->value_type;
	std::shared_ptr<void> result;

	bool l_v = (left->node_type == NODE_TYPE::VALUE), r_v = (right->node_type == NODE_TYPE::VALUE);

	switch (node_type)
	{
	case NODE_TYPE::ADDITION:
	case NODE_TYPE::SUBSTRACTION:
	case NODE_TYPE::MULTYPLICATION:
	case NODE_TYPE::DIVISION:       value_type = Integer; ptr_ = arithm_op(x, y, l_v, r_v, node_type); break;
	case NODE_TYPE::EQUAL:			value_type = Boolean; ptr_ = equal(x, y, l_v, r_v, lt); break;
	default: ptr_ = std::make_shared<std::vector<int>>();
	}
	return ptr_;
}

void Evaluator::print()
{
	for (auto& [k, v]: TOKEN_TO_OPERATOR)
	{
		if (v == node_type)
		{
			std::cout << k << " ";
		}
	}
	if (node_type == NODE_TYPE::COLUMN)
	{
		std::cout << "column " << TypeNames[value_type] << " ";
	}
	if (node_type == NODE_TYPE::VALUE)
	{
		std::cout << "value " << TypeNames[value_type] << " ";
	}
}


int plus(int a, int b) {return a+b;}
int minus(int a, int b) {return a-b;}
int multyply(int a, int b) {return a*b;}
int divide(int a, int b) {return a/b;}

std::map<NODE_TYPE, int(*)(int,int)> math_ops =
{
	{NODE_TYPE::ADDITION, plus},
	{NODE_TYPE::SUBSTRACTION, minus},
	{NODE_TYPE::MULTYPLICATION, multyply},
	{NODE_TYPE::DIVISION, divide}
};


#define VECI(x) std::static_pointer_cast<std::vector<int>>(x)
#define VALI(x) std::static_pointer_cast<int>(x)
#define VECT(x, t) std::static_pointer_cast<std::vector<t>>(x)
#define VALT(x, t) std::static_pointer_cast<t>(x)

std::shared_ptr<std::vector<int>> arithm_op(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE n_type)
{
	std::cout << "arithm_op " << l_v << " " << r_v << "\n";
	size_t sz; if (l_v && r_v) {sz = 1;} else if (l_v) {sz = VECI(y)->size();} else if (r_v) {sz = VECI(x)->size();} else {sz = VECI(x)->size();}
	auto z = std::vector<int>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		int a = (l_v) ? *VALI(x) : (*VECI(x))[i];
		int b = (r_v) ? *VALI(y) : (*VECI(y))[i];
		z[i] = math_ops[n_type](a, b);
		std::cout << a << " " << b << " " << z[i] << "\n";
	}
	return std::make_shared<std::vector<int>>(std::move(z));
}


template<typename T>
std::shared_ptr<std::vector<bool>> compare(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v)
{
	size_t sz; if (l_v && r_v) {sz = 1;} else if (l_v) {sz = VECT(y, T)->size();} else if (r_v) {sz = VECT(x, T)->size();} else {sz = VECT(x, T)->size();}
	auto z = std::vector<bool>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		T a = (l_v) ? *VALT(x, T) : (*VECT(x, T))[i];
		T b = (r_v) ? *VALT(y, T) : (*VECT(y, T))[i];
		z[i] = (a == b);
		std::cout << "eq: " << z[i] << "\n";
	}

	return std::make_shared<std::vector<bool>>(std::move(z));
}


std::shared_ptr<std::vector<bool>> equal(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, Types type)
{
	if (type == Integer)
	{
		return compare<int>(x, y, l_v, r_v);
	}
	else if (type == Boolean)
	{
		return compare<bool>(x, y, l_v, r_v);
	}
	else
	{
		return compare<std::string>(x, y, l_v, r_v);
	}
}