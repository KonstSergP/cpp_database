#pragma once
#include <memory>
#include "utils.h"
#include "table.h"


enum class NODE_TYPE
{
	NOPE,
	COLUMN,
	VALUE,
	ADDITION,
	SUBSTRACTION,
	MULTYPLICATION,
	DIVISION,
	MOD,
	LESS,
	EQUAL,
	GREATER,
	LE,
	GE,
	N_EQUAL,
	AND,
	OR,
	NOT,
	XOR,
	ABS,
	OPEN_BRACE,
	HEAD
};

int prior(NODE_TYPE type);
extern std::map<std::string, NODE_TYPE> TOKEN_TO_OPERATOR;

class Evaluator
{
public:
	NODE_TYPE node_type;
	Types value_type;
	std::shared_ptr<void> ptr_;
	std::shared_ptr<Evaluator> left, right;

	Evaluator() = default;
	Evaluator(std::shared_ptr<void> ptr, NODE_TYPE type);
	Evaluator(std::shared_ptr<void> ptr, NODE_TYPE type, Types val_type);
	void print();
	virtual std::shared_ptr<void> evaluate(std::shared_ptr<std::vector<bool>> vc);
};


class EvaluatorHead: public Evaluator
{
public:
	EvaluatorHead() = default;
	std::shared_ptr<void> evaluate(std::shared_ptr<std::vector<bool>> vc) override;
};
