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
	memdb::Database db, db1;

	auto result = db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
	db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");
	db.execute("create table third ({unique} is_true: bool = false, is_false: bool = true)");

	std::cout << result.is_ok() << "\n";

	db.describe();
	std::cout << "\n\n\n\n\n";

	for (int i = 0; i < 30; i++)
	{
		std::string query = "INSERT (login = \"" + random_string(15) + "\", password_hash = 0x" + random_string(10) + ") to first";
		db.execute(query);
	}
	auto res = db.execute("INSERT (200, \"kostya\", 0xaaaabbbb, true) to first");

	
	db.save_to_file(std::ofstream("db.bin", std::ios::binary));

	printf("LOADING\n");

	db1.load_from_file(std::ifstream("db.bin", std::ios::binary));
	db1.describe();

	for (auto& row: db1.get("first"))
	{
		std::cout << "Val: " << row.get<int32_t>("id") << " " << row.get<std::string>("login") << " " << row.get<std::string>("password_hash") << " " << row.get<bool>("is_admin") << "\n";
	}
	printf("\n\n");
	memdb::Database db3;
    auto r1 = db3.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    auto r2 = db3.execute("INSERT (1,,0xAAAB,) to first");
    //r2.what();
    auto r3 = db3.execute("INSERT (,\"testing\",0xAAAB,) to first");
    auto r4 = db3.execute("INSERT (2,\"test_string\",0x12345,true) to first");
    auto r5 = db3.execute("INSERT (,\"abcd\",0xCCCD,false) to first");
    for (auto& row: db3.get("first"))
    {
    	std::cout << "Val: " << row.get<int32_t>("id") << " " << row.get<std::string>("login") << " " << row.get<std::string>("password_hash") << " " << row.get<bool>("is_admin") << "\n";
    }


	return 0;
}

// find . -name '*.cpp' -type f -print0 | xargs -0 cat | wc -l
