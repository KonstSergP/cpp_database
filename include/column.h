#pragma once

#include "utils.h"
#include "column.h"
#include "ErrorHandler.h"
#include <vector>
#include <map>
#include <memory>
#include <fstream>



namespace Columns
{

	struct ColumnAttributes
	{
		bool key = false;
		bool unique = false;
		bool autoincrement = false;

		ColumnAttributes() = default;
		ColumnAttributes(bool k, bool u, bool a): key(k), unique(u), autoincrement(a)
		{}
	};



	class Column
	{
	public:
		std::string name;
		TypeInfo info;
		ColumnAttributes attributes;
		
		void set_name(std::string value);
		void set_attributes(bool k, bool u, bool a);
		void set_attributes(ColumnAttributes attrs);
		bool is_unique() const;
		bool is_autoinc() const;
		bool is_key() const;
		void describe() const;
		std::string get_type_string() const;
		std::shared_ptr<Column> get_structure();
		static std::shared_ptr<Column> load_from_file(std::ifstream& in, int rws);

		virtual void set_default(std::shared_ptr<void> ptr) = 0;
		virtual std::shared_ptr<void> get_default() = 0;
		virtual void change_default(std::shared_ptr<Column> other) = 0;
		virtual bool can_be_added(std::shared_ptr<Column> other) = 0;
		virtual void add_values(std::shared_ptr<Column> other) = 0;
		virtual void add_value(std::shared_ptr<void>& ptr) = 0;
		virtual std::shared_ptr<void> get_values() = 0;
		virtual void set_values(std::shared_ptr<void> ptr) = 0;
		virtual std::shared_ptr<void> extract(std::shared_ptr<std::vector<bool>> vec) = 0;
		virtual void replace(std::shared_ptr<void> vals, std::shared_ptr<std::vector<bool>> bools) = 0;
		virtual void save_to_file(std::ofstream& out) = 0;
		virtual void load_values(std::ifstream& in, int rws) = 0;

		template <typename T>
		T get(int index)
		{
			auto vec = std::static_pointer_cast<std::vector<T>>(get_values());
			return (*vec)[index];
		}

		template <typename T>
		bool is_intersecting(std::shared_ptr<std::vector<T>> left, std::shared_ptr<std::vector<T>> right)
		{
			int szl = left->size();
			int szr = right->size();
			for (int i = 0; i < szl; i++)
			{
				for (int j = 0; j < szr; j++)
				{
					if ((*left)[i] == (*right)[j]) {
						return true;
					}
				}
			}
			return false;
		}
	};


	class IntColumn: public Column
	{
		std::shared_ptr<std::vector<int>> vec_;
		std::shared_ptr<int> default_value_;

	public:

		IntColumn();
		~IntColumn() = default;
		void set_default(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> get_default() override;
		void change_default(std::shared_ptr<Column> other) override;
		bool can_be_added(std::shared_ptr<Column> other) override;
		void add_values(std::shared_ptr<Column> other) override;
		void add_value(std::shared_ptr<void>& ptr) override;
		std::shared_ptr<void> get_values() override;
		void set_values(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> extract(std::shared_ptr<std::vector<bool>> vec) override;
		void replace(std::shared_ptr<void> vals, std::shared_ptr<std::vector<bool>> bools) override;
		void save_to_file(std::ofstream& out) override;
		void load_values(std::ifstream& in, int rws) override;
	};


	class BoolColumn: public Column
	{
		std::shared_ptr<std::vector<bool>> vec_;
		std::shared_ptr<bool> default_value_;

	public:

		BoolColumn();
		~BoolColumn() = default;
		void set_default(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> get_default() override;
		void change_default(std::shared_ptr<Column> other) override;
		bool can_be_added(std::shared_ptr<Column> other) override;
		void add_values(std::shared_ptr<Column> other) override;
		void add_value(std::shared_ptr<void>& ptr) override;
		std::shared_ptr<void> get_values() override;
		void set_values(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> extract(std::shared_ptr<std::vector<bool>> vec) override;
		void replace(std::shared_ptr<void> vals, std::shared_ptr<std::vector<bool>> bools) override;
		void save_to_file(std::ofstream& out) override;
		void load_values(std::ifstream& in, int rws) override;
	};

	class TextColumn: public Column
	{
		std::shared_ptr<std::vector<std::string>> vec_;
		std::shared_ptr<std::string> default_value_;

	public:

		TextColumn();
		~TextColumn() = default;
		void set_default(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> get_default() override;
		void change_default(std::shared_ptr<Column> other) override;
		bool can_be_added(std::shared_ptr<Column> other) override;
		void add_values(std::shared_ptr<Column> other) override;
		void add_value(std::shared_ptr<void>& ptr) override;
		std::shared_ptr<void> get_values() override;
		void set_values(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> extract(std::shared_ptr<std::vector<bool>> vec) override;
		void replace(std::shared_ptr<void> vals, std::shared_ptr<std::vector<bool>> bools) override;
		void save_to_file(std::ofstream& out) override;
		void load_values(std::ifstream& in, int rws) override;
	};

	class BytesColumn: public Column
	{
		std::shared_ptr<std::vector<std::string>> vec_;
		std::shared_ptr<std::string> default_value_;

	public:

		BytesColumn();
		~BytesColumn() = default;
		void set_default(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> get_default() override;
		void change_default(std::shared_ptr<Column> other) override;
		bool can_be_added(std::shared_ptr<Column> other) override;
		void add_values(std::shared_ptr<Column> other) override;
		void add_value(std::shared_ptr<void>& ptr) override;
		std::shared_ptr<void> get_values() override;
		void set_values(std::shared_ptr<void> ptr) override;
		std::shared_ptr<void> extract(std::shared_ptr<std::vector<bool>> vec) override;
		void replace(std::shared_ptr<void> vals, std::shared_ptr<std::vector<bool>> bools) override;
		void save_to_file(std::ofstream& out) override;
		void load_values(std::ifstream& in, int rws) override;
	};

	std::shared_ptr<Column> CreateColumn(TypeInfo type_info);
};
