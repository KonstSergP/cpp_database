#include "../include/column.h"
#include "../include/utils.h"
#include "../include/ErrorHandler.h"
#include <iostream>

using namespace Columns;

void Column::set_attributes(bool k, bool u, bool a) {
	attributes = ColumnAttributes(k, u, a);
}


void Column::set_attributes(ColumnAttributes attrs) {
	attributes = attrs;
}


void Column::set_name(std::string value) {
	name = value;
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


bool Column::is_unique() const {
	return attributes.unique;
}


bool Column::is_autoinc() const {
	return attributes.autoincrement;
}


bool Column::is_key() const {
	return attributes.key;
}


std::shared_ptr<Column> Columns::CreateColumn(TypeInfo type_info)
{
	std::shared_ptr<Column> colptr;
	switch (type_info.type)
	{
	case Integer:
		colptr = std::make_shared<IntColumn>();
		break;
	case Boolean:
		colptr = std::make_shared<BoolColumn>();
		break;
	case Text:
		colptr = std::make_shared<TextColumn>();
		break;
	case Bytes:
		colptr = std::make_shared<BytesColumn>();
		break;
	default:
		throw TableException(TE::UNKNOWN_TYPE);
	}
	colptr->info = type_info;
	return colptr;
}


IntColumn::IntColumn() {
	vec_ = std::make_shared<std::vector<int>>();
}

BoolColumn::BoolColumn() {
	vec_ = std::make_shared<std::vector<bool>>();
}

TextColumn::TextColumn() {
	vec_ = std::make_shared<std::vector<std::string>>();
}

BytesColumn::BytesColumn() {
	vec_ = std::make_shared<std::vector<std::string>>();
}


std::shared_ptr<Column> Column::get_structure()
{
	auto column = CreateColumn(info);
	column->set_name(name);
	column->set_attributes(attributes);
	column->set_default(get_default());
	return column;
}

#define VALT(x, t) std::static_pointer_cast<t>(x)
void IntColumn::set_default(std::shared_ptr<void> ptr) {
	default_value_ = VALT(ptr, int);
}

void BoolColumn::set_default(std::shared_ptr<void> ptr) {
	default_value_ = VALT(ptr, bool);
}

void TextColumn::set_default(std::shared_ptr<void> ptr) {
	default_value_ = VALT(ptr, std::string);
}

void BytesColumn::set_default(std::shared_ptr<void> ptr) {
	default_value_ = VALT(ptr, std::string);
}


std::shared_ptr<void> IntColumn::get_default()
{
	if (!default_value_) {return std::shared_ptr<int>();}
	return std::make_shared<int>(*default_value_);
}

std::shared_ptr<void> BoolColumn::get_default()
{
	if (!default_value_) {return std::shared_ptr<bool>();}
	return std::make_shared<bool>(*default_value_);
}

std::shared_ptr<void> TextColumn::get_default()
{
	if (!default_value_) {return std::shared_ptr<std::string>();}
	return std::make_shared<std::string>(*default_value_);
}

std::shared_ptr<void> BytesColumn::get_default()
{
	if (!default_value_) {return std::shared_ptr<std::string>();}
	return std::make_shared<std::string>(*default_value_);
}


void IntColumn::change_default(std::shared_ptr<Column> other)
{
	auto a = std::static_pointer_cast<IntColumn>(other);
	default_value_ = std::make_shared<int>(*(a->default_value_));
}

void BoolColumn::change_default(std::shared_ptr<Column> other)
{
	auto a = std::static_pointer_cast<BoolColumn>(other);
	default_value_ = std::make_shared<bool>(*(a->default_value_));
}

void TextColumn::change_default(std::shared_ptr<Column> other)
{
	auto a = std::static_pointer_cast<TextColumn>(other);
	default_value_ = std::make_shared<std::string>(*(a->default_value_));
}

void BytesColumn::change_default(std::shared_ptr<Column> other)
{
	auto a = std::static_pointer_cast<BytesColumn>(other);
	default_value_ = std::make_shared<std::string>(*(a->default_value_));
}


bool IntColumn::can_be_added(std::shared_ptr<Column> other)
{
	if (!is_unique()) {return true;}
	auto a = std::static_pointer_cast<IntColumn>(other);
	bool intersect = is_intersecting<int>(vec_, a->vec_);
	return !intersect;
}

bool BoolColumn::can_be_added(std::shared_ptr<Column> other)
{
	if (!is_unique()) {return true;}
	auto a = std::static_pointer_cast<BoolColumn>(other);
	bool intersect = is_intersecting<bool>(vec_, a->vec_);
	return !intersect;
}

bool TextColumn::can_be_added(std::shared_ptr<Column> other)
{
	if (!is_unique()) {return true;}
	auto a = std::static_pointer_cast<TextColumn>(other);
	bool intersect = is_intersecting<std::string>(vec_, a->vec_);
	return !intersect;
}

bool BytesColumn::can_be_added(std::shared_ptr<Column> other)
{
	if (!is_unique()) {return true;}
	auto a = std::static_pointer_cast<BytesColumn>(other);
	bool intersect = is_intersecting<std::string>(vec_, a->vec_);
	return !intersect;
}


void IntColumn::add_values(std::shared_ptr<Column> other)
{
	auto right = std::static_pointer_cast<IntColumn>(other);
	std::copy(right->vec_->begin(), right->vec_->end(), std::back_inserter(*vec_));
}

void BoolColumn::add_values(std::shared_ptr<Column> other)
{
	auto right = std::static_pointer_cast<BoolColumn>(other);
	std::copy(right->vec_->begin(), right->vec_->end(), std::back_inserter(*vec_));
}

void TextColumn::add_values(std::shared_ptr<Column> other)
{
	auto right = std::static_pointer_cast<TextColumn>(other);
	std::copy(right->vec_->begin(), right->vec_->end(), std::back_inserter(*vec_));
}

void BytesColumn::add_values(std::shared_ptr<Column> other)
{
	auto right = std::static_pointer_cast<BytesColumn>(other);
	std::copy(right->vec_->begin(), right->vec_->end(), std::back_inserter(*vec_));
}


void IntColumn::add_value(std::shared_ptr<void>& ptr)
{
	auto val = std::static_pointer_cast<int>(ptr);
	vec_->push_back(*val);
}

void BoolColumn::add_value(std::shared_ptr<void>& ptr)
{
	auto val = std::static_pointer_cast<bool>(ptr);
	vec_->push_back(*val);
}

void TextColumn::add_value(std::shared_ptr<void>& ptr)
{
	auto val = std::static_pointer_cast<std::string>(ptr);
	vec_->push_back(*val);
}

void BytesColumn::add_value(std::shared_ptr<void>& ptr)
{
	auto val = std::static_pointer_cast<std::string>(ptr);
	vec_->push_back(*val);
}


std::shared_ptr<void> IntColumn::get_values() {
	return vec_;
}

std::shared_ptr<void> BoolColumn::get_values() {
	return vec_;
}

std::shared_ptr<void> TextColumn::get_values() {
	return vec_;
}

std::shared_ptr<void> BytesColumn::get_values() {
	return vec_;
}


void IntColumn::set_values(std::shared_ptr<void> ptr) {
	vec_ = std::static_pointer_cast<std::vector<int>>(ptr);
}

void BoolColumn::set_values(std::shared_ptr<void> ptr) {
	vec_ = std::static_pointer_cast<std::vector<bool>>(ptr);
}

void TextColumn::set_values(std::shared_ptr<void> ptr) {
	vec_ = std::static_pointer_cast<std::vector<std::string>>(ptr);
}

void BytesColumn::set_values(std::shared_ptr<void> ptr) {
	vec_ = std::static_pointer_cast<std::vector<std::string>>(ptr);
}


template<typename T>
static std::pair<std::shared_ptr<std::vector<T>>, std::shared_ptr<std::vector<T>>> 
		split_vecs(std::shared_ptr<std::vector<T>> src, std::shared_ptr<std::vector<bool>> vec)
{
	auto l = std::make_shared<std::vector<T>>();
	auto r = std::make_shared<std::vector<T>>();
	for (size_t i = 0; i < vec->size(); i++)
	{
		if ((*vec)[i]) {
			l->push_back((*src)[i]);
		}else {
			r->push_back((*src)[i]);
		}
	}
	return {l, r};
}


std::shared_ptr<void> IntColumn::extract(std::shared_ptr<std::vector<bool>> vec)
{
	auto [a, b] = split_vecs<int>(vec_, vec);
	vec_ = b;
	return a;
}

std::shared_ptr<void> BoolColumn::extract(std::shared_ptr<std::vector<bool>> vec)
{
	auto [a, b] = split_vecs<bool>(vec_, vec);
	vec_ = b;
	return a;
}

std::shared_ptr<void> TextColumn::extract(std::shared_ptr<std::vector<bool>> vec)
{
	auto [a, b] = split_vecs<std::string>(vec_, vec);
	vec_ = b;
	return a;
}

std::shared_ptr<void> BytesColumn::extract(std::shared_ptr<std::vector<bool>> vec)
{
	auto [a, b] = split_vecs<std::string>(vec_, vec);
	vec_ = b;
	return a;
}
