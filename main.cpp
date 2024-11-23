#include <iostream>
#include <format>
#include "include/database.h"

std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEF"
        "abcdef";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}



int main()
{
	memdb::Database db;

	auto result = db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
	db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");
	db.execute("create table third ({unique} is_true: bool = false, is_false: bool = true)");

	std::cout << result.is_ok() << "\n";

	db.describe();
	std::cout << "\n\n\n\n\n";

	for (int i = 0; i < 100; i++)
	{
		std::string query = "INSERT (login = \"" + random_string(15) + "\", password_hash = 0x" + random_string(10) + ") to first";
		db.execute(query);
	}

	for (auto& row: db.get("first"))
	{
		std::cout << "Val: " << row.get<int32_t>("id") << " " << row.get<std::string>("login") << " " << row.get<std::string>("password_hash") << " " << row.get<bool>("is_admin") << "\n";
	}

	

	

	return 0;
}

// find . -name '*.cpp' -type f -print0 | xargs -0 cat | wc -l
