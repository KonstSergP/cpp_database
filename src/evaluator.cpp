#include <iostream>
#include "../include/utils.h"
#include "../include/evaluator.h"
#include "../include/ErrorHandler.h"
#include <map>


std::map<std::string, NODE_TYPE> TOKEN_TO_OPERATOR = 
{
	{"+",  NODE_TYPE::ADDITION},
	{"-",  NODE_TYPE::SUBSTRACTION},
	{"*",  NODE_TYPE::MULTYPLICATION},
	{"/",  NODE_TYPE::DIVISION},
	{"%",  NODE_TYPE::MOD},
	{"<",  NODE_TYPE::LESS},
	{"=",  NODE_TYPE::EQUAL},
	{">",  NODE_TYPE::GREATER},
	{"<=", NODE_TYPE::LE},
	{">=", NODE_TYPE::GE},
	{"!=", NODE_TYPE::N_EQUAL},
	{"&&", NODE_TYPE::AND},
	{"||", NODE_TYPE::OR},
	{"!",  NODE_TYPE::NOT},
	{"^^", NODE_TYPE::XOR},
	{"|",  NODE_TYPE::ABS},
	{"(",  NODE_TYPE::OPEN_BRACE},
	{"&&", NODE_TYPE::AND},
	{"||", NODE_TYPE::OR},
	{"^^", NODE_TYPE::XOR}
};


int prior(NODE_TYPE type)
{
	switch(type)
	{
	case NODE_TYPE::NOT:
		return 2;
    case NODE_TYPE::MULTYPLICATION:
    case NODE_TYPE::DIVISION:
    case NODE_TYPE::MOD:
        return 3;
    case NODE_TYPE::SUBSTRACTION:
    case NODE_TYPE::ADDITION:
        return 4;
    case NODE_TYPE::LESS:
    case NODE_TYPE::GREATER:
    case NODE_TYPE::LE:
    case NODE_TYPE::GE:
    	return 6;
    case NODE_TYPE::EQUAL:
    case NODE_TYPE::N_EQUAL:
    	return 7;
    case NODE_TYPE::XOR:
    	return 9;
    case NODE_TYPE::AND:
    	return 11;
    case NODE_TYPE::OR:
    	return 12;
    case NODE_TYPE::OPEN_BRACE:
    case NODE_TYPE::ABS:
        return 20;
    default:
    	return 0;
	}
}

static std::shared_ptr<void> arithm_op(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE n_type, Evaluator* pt);
static std::shared_ptr<void> comparison(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, Types type, NODE_TYPE tp, Evaluator* pt);
static std::shared_ptr<void> logical_op(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE n_type, Evaluator* pt);
static std::shared_ptr<void> concat(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, Evaluator* pt);
static std::shared_ptr<void> logical_not(std::shared_ptr<void> x, bool l_v, Evaluator* pt);
static std::shared_ptr<void> length(std::shared_ptr<void> x, bool l_v, Evaluator* pt);

Evaluator::Evaluator(std::shared_ptr<void> ptr, NODE_TYPE type): node_type(type), ptr_(ptr)
{}


Evaluator::Evaluator(std::shared_ptr<void> ptr, NODE_TYPE type, Types val_type): node_type(type), value_type(val_type), ptr_(ptr)
{}


template<typename T>
static std::shared_ptr<void> choose(std::shared_ptr<void> col, std::shared_ptr<std::vector<bool>> vc)
{
	auto a = std::static_pointer_cast<std::vector<T>>(col);
	auto res = std::make_shared<std::vector<T>>();
	for (size_t i = 0; i < vc->size(); i++)
	{
		if ((*vc)[i])
		{
			res->push_back((*a)[i]);
		}
	}
	return res;
}


std::shared_ptr<void> Evaluator::evaluate(std::shared_ptr<std::vector<bool>> vc)
{
	if (node_type == NODE_TYPE::VALUE) {
		return ptr_;
	}
	if (node_type == NODE_TYPE::COLUMN)
	{
		if (!vc) {return ptr_;}
		switch (value_type)
		{
		case Integer: ptr_ = choose<int>(ptr_, vc); break;
		case Boolean: ptr_ = choose<bool>(ptr_, vc); break;
		case Text:    ptr_ = choose<std::string>(ptr_, vc); break;
		case Bytes:   ptr_ = choose<std::string>(ptr_, vc); break;
		}
		return ptr_;
	}

	auto x = left->evaluate(vc); 	Types lt = left->value_type;
	auto y = (node_type != NODE_TYPE::NOT && node_type != NODE_TYPE::ABS) ? right->evaluate(vc) : nullptr; //Types rt = right->value_type;
	std::shared_ptr<void> result;

	bool l_v = (left->node_type == NODE_TYPE::VALUE), r_v = (node_type != NODE_TYPE::NOT && node_type != NODE_TYPE::ABS) ? (right->node_type == NODE_TYPE::VALUE) : 0;

	switch (node_type)
	{
	case NODE_TYPE::ADDITION:
									if (left->value_type == Text) {value_type = Text; ptr_ = concat(x, y, l_v, r_v, this); break;} [[fallthrough]];
	case NODE_TYPE::SUBSTRACTION:
	case NODE_TYPE::MULTYPLICATION:
	case NODE_TYPE::DIVISION:
	case NODE_TYPE::MOD:            value_type = Integer; ptr_ = arithm_op(x, y, l_v, r_v, node_type, this); break;
	case NODE_TYPE::EQUAL:
	case NODE_TYPE::N_EQUAL:
	case NODE_TYPE::LESS:
	case NODE_TYPE::GREATER:
	case NODE_TYPE::LE:
	case NODE_TYPE::GE:  			value_type = Boolean; ptr_ = comparison(x, y, l_v, r_v, lt, node_type, this); break;
	case NODE_TYPE::AND:
	case NODE_TYPE::OR:
	case NODE_TYPE::XOR:			value_type = Boolean; ptr_ = logical_op(x, y, l_v, r_v, node_type, this); break;
	case NODE_TYPE::NOT:			value_type = Boolean; ptr_ = logical_not(x, l_v, this); break;
	case NODE_TYPE::ABS:			value_type = Integer; ptr_ = length(x, l_v, this); break;
	default: value_type = Integer; ptr_ = std::make_shared<std::vector<int>>();
	}
	return ptr_;
}


template<typename T>
static std::shared_ptr<std::vector<T>> repeat(std::shared_ptr<void> val, size_t n)
{
	auto a = std::make_shared<std::vector<T>>();
	auto value = std::static_pointer_cast<T>(val);
	for (size_t i = 0; i < n; i++) {
		a->push_back(*value);
	}
	return a;
}


std::shared_ptr<void> EvaluatorHead::evaluate(std::shared_ptr<std::vector<bool>> vc)
{
	try {
		ptr_ = left->evaluate(vc);
	} catch (std::exception& e) {
		throw TableException(TE::EVAL_FAULT);
	}
	value_type = left->value_type;
	if (left->node_type == NODE_TYPE::VALUE)
	{
		size_t sz;
		if (!vc) {sz = rows;} else {sz = vc->size();}
		switch (value_type)
		{
		case Integer: ptr_ = repeat<int>(ptr_, sz); break;
		case Boolean: ptr_ = repeat<bool>(ptr_, sz); break;
		case Text:    ptr_ = repeat<std::string>(ptr_, sz); break;
		case Bytes:   ptr_ = repeat<std::string>(ptr_, sz); break;
		}
	}
	return ptr_;
}


void Evaluator::print()
{
	for (auto& [k, v]: TOKEN_TO_OPERATOR)
	{
		if (v == node_type) {
			std::cout << k << " ";
		}
	}
	if (node_type == NODE_TYPE::COLUMN) {
		std::cout << "column " << TypeNames[value_type] << " ";
	}
	if (node_type == NODE_TYPE::VALUE) {
		std::cout << "value " << TypeNames[value_type] << " ";
	}
}


static int plus(int a, int b) {return a+b;}
static int minus(int a, int b) {return a-b;}
static int multyply(int a, int b) {return a*b;}
static int divide(int a, int b) {return a/b;}
static int mod(int a, int b) {return a%b;}

std::map<NODE_TYPE, int(*)(int,int)> math_ops =
{
	{NODE_TYPE::ADDITION, plus},
	{NODE_TYPE::SUBSTRACTION, minus},
	{NODE_TYPE::MULTYPLICATION, multyply},
	{NODE_TYPE::DIVISION, divide},
	{NODE_TYPE::MOD, mod}
};


#define VECI(x) std::static_pointer_cast<std::vector<int>>(x)
#define VALI(x) std::static_pointer_cast<int>(x)
#define VECT(x, t) std::static_pointer_cast<std::vector<t>>(x)
#define VALT(x, t) std::static_pointer_cast<t>(x)

static std::shared_ptr<void> arithm_op(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE n_type, Evaluator* pt)
{
	size_t sz; if (l_v && r_v) {sz = 1;} else if (l_v) {sz = VECI(y)->size();} else if (r_v) {sz = VECI(x)->size();} else {sz = VECI(x)->size();}
	auto z = std::vector<int>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		int a = (l_v) ? *VALI(x) : (*VECI(x))[i];
		int b = (r_v) ? *VALI(y) : (*VECI(y))[i];
		z[i] = math_ops[n_type](a, b);
	}
	if (sz == 1)
	{
		pt->node_type = NODE_TYPE::VALUE;
		return std::make_shared<int>(z[0]);
	}else
	{
		return std::make_shared<std::vector<int>>(std::move(z));
	}
}


template<typename T>
static std::shared_ptr<void> compare(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE tp, Evaluator* pt)
{
	size_t sz; if (l_v && r_v) {sz = 1;} else if (l_v) {sz = VECT(y, T)->size();} else if (r_v) {sz = VECT(x, T)->size();} else {sz = VECT(x, T)->size();}
	auto z = std::vector<bool>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		T a = (l_v) ? *VALT(x, T) : (*VECT(x, T))[i];
		T b = (r_v) ? *VALT(y, T) : (*VECT(y, T))[i];

		switch (tp)
		{
		case NODE_TYPE::EQUAL:
			z[i] = (a == b); break;
		case NODE_TYPE::N_EQUAL:
			z[i] = (a != b); break;
		case NODE_TYPE::LESS:
			z[i] = (a < b); break;
		case NODE_TYPE::GREATER:
			z[i] = (a > b); break;
		case NODE_TYPE::LE:
			z[i] = (a <= b); break;
		case NODE_TYPE::GE:
			z[i] = (a >= b); break;
		default: break;
		}
	}

	if (sz == 1)
	{
		pt->node_type = NODE_TYPE::VALUE;
		return std::make_shared<bool>(z[0]);
	}else
	{
		return std::make_shared<std::vector<bool>>(std::move(z));
	}
}


static std::shared_ptr<void> comparison(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, Types type, NODE_TYPE tp, Evaluator* pt)
{
	if (type == Integer) {
		return compare<int>(x, y, l_v, r_v, tp, pt);
	}
	else if (type == Boolean) {
		return compare<bool>(x, y, l_v, r_v, tp, pt);
	}
	else {
		return compare<std::string>(x, y, l_v, r_v, tp, pt);
	}
}


static bool func_and(bool a, bool b) {return a && b;}
static bool func_or(bool a, bool b) {return a || b;}
static bool func_xor(bool a, bool b) {return a ^ b;}

std::map<NODE_TYPE, bool(*)(bool,bool)> log_ops =
{
	{NODE_TYPE::AND, func_and},
	{NODE_TYPE::OR, func_or},
	{NODE_TYPE::XOR, func_xor}
};


std::shared_ptr<void> logical_op(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, NODE_TYPE n_type, Evaluator* pt)
{
	size_t sz; if (l_v && r_v) {sz = 1;} else if (l_v) {sz = VECT(y, bool)->size();} else if (r_v) {sz = VECT(x, bool)->size();} else {sz = VECT(x, bool)->size();}
	auto z = std::vector<bool>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		bool a = (l_v) ? *VALT(x, bool) : (*VECT(x, bool))[i];
		bool b = (r_v) ? *VALT(y, bool) : (*VECT(y, bool))[i];
		z[i] = log_ops[n_type](a, b);
	}
	if (sz == 1)
	{
		pt->node_type = NODE_TYPE::VALUE;
		return std::make_shared<bool>(z[0]);
	}else
	{
		return std::make_shared<std::vector<bool>>(std::move(z));
	}
}


std::shared_ptr<void> concat(std::shared_ptr<void> x, std::shared_ptr<void> y, bool l_v, bool r_v, Evaluator* pt)
{
	size_t sz; if (l_v && r_v) {sz = 1;} else if (l_v) {sz = VECT(y, std::string)->size();} else if (r_v) {sz = VECT(x, std::string)->size();} else {sz = VECT(x, std::string)->size();}
	auto z = std::vector<std::string>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		std::string a = (l_v) ? *VALT(x, std::string) : (*VECT(x, std::string))[i];
		std::string b = (r_v) ? *VALT(y, std::string) : (*VECT(y, std::string))[i];
		z[i] = a+b;
	}
	if (sz == 1)
	{
		pt->node_type = NODE_TYPE::VALUE;
		return std::make_shared<std::string>(z[0]);
	}else
	{
		return std::make_shared<std::vector<std::string>>(std::move(z));
	}
}


std::shared_ptr<void> logical_not(std::shared_ptr<void> x, bool l_v, Evaluator* pt)
{
	size_t sz; if (l_v) {sz = 1;} else {sz = VECT(x, bool)->size();}
	auto z = std::vector<bool>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		bool a = (l_v) ? *VALT(x, bool) : (*VECT(x, bool))[i];
		z[i] = !a;
	}
	if (sz == 1)
	{
		pt->node_type = NODE_TYPE::VALUE;
		return std::make_shared<bool>(z[0]);
	}else
	{
		return std::make_shared<std::vector<bool>>(std::move(z));
	}
}


std::shared_ptr<void> length(std::shared_ptr<void> x, bool l_v, Evaluator* pt)
{
	size_t sz; if (l_v) {sz = 1;} else {sz = VECT(x, std::string)->size();}
	auto z = std::vector<int>(sz);
	for (size_t i = 0; i < sz; i++)
	{
		z[i] = (l_v) ? VALT(x, std::string)->size() : (*VECT(x, std::string))[i].size();;
	}
	if (sz == 1)
	{
		pt->node_type = NODE_TYPE::VALUE;
		return std::make_shared<int>(z[0]);
	}else
	{
		return std::make_shared<std::vector<int>>(std::move(z));
	}
}


void EvaluatorHead::set_rows(int rws)
{
	rows = rws;
}
