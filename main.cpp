#include <iostream>

#define DISABLE_INDEX
#include "include/database.hpp"




int main()
{
	memdb::Database db;

	auto result = db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false)");
	result = db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");
	db.execute("create table third ({unique} is_true: bool = false, is_false: bool = true)");

	std::cout << result.is_ok() << "\n";

	db.describe();
	std::cout << "\n\n\n\n\n";

	auto res = db.execute("INSERT (id = 11, login = \"amogus\", password_hash = \"amogus\") to first");
	db.execute("INSERT (4, \"kosepetrov\", \"amogusich\", true) to first");

	for (auto& row: db.get_table("first"))
	{
		std::cout << "Val: " << row.get<int32_t>("id") << " " << row.get<std::string>("login") << " " << row.get<std::string>("password_hash") << " " << row.get<bool>("is_admin") << "\n";
	}

	
	return 0;
}

// find . -name '*.cpp' -type f -print0 | xargs -0 cat | wc -l
