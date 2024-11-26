#include "./gtest/gtest.h"
#include "include/database.h"
#include <string>

using namespace std;
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

TEST(TestCreateTable, Subtest_1)
{
    memdb::Database db;
    auto r1 = db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    auto r2 = db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {key, unique} name: string[16] = \"amogus\")");
    auto r3 = db.execute("create table third ({unique} is_true: bool = false, is_false: bool = true)");
    ASSERT_TRUE(r1.is_ok() && r2.is_ok() && r3.is_ok());
}

TEST(TestInsert, Subtest_1)
{
    memdb::Database db;
    auto r1 = db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    auto r2 = db.execute("INSERT (1,,0xAAAB,) to first");
    //r2.what();
    auto r3 = db.execute("INSERT (,\"testing\",0xAAAB,) to first");
    auto r4 = db.execute("INSERT (,\"test_string\",0x12345,true) to first");
    auto r5 = db.execute("INSERT (,\"abcd\",0xCCCD,false) to first");

    ASSERT_TRUE(r1.is_ok());
    ASSERT_TRUE(r2.is_ok());
    ASSERT_TRUE(r3.is_ok());
    ASSERT_TRUE(r4.is_ok());
    ASSERT_TRUE(r5.is_ok());

    auto r6 = db.execute("INSERT (id = 11,password_hash = 0xAAAB, login = \"kostya\",is_admin = false) to first");
    auto r7 = db.execute("INSERT (id = 30, password_hash = 0xABBB, login = \"login\", is_admin = false) to first");
    auto r8 = db.execute("INSERT (id = 20,login = \"test_string1234\",password_hash = 0x12345,is_admin = true) to first");
    auto r9 = db.execute("INSERT (is_admin = true, password_hash = 0xCCCD, login = \"petrov\", id = 40) to first");
    ASSERT_TRUE(r6.is_ok());
    ASSERT_TRUE(r7.is_ok());
    ASSERT_TRUE(r8.is_ok());
    ASSERT_TRUE(r9.is_ok());
}

TEST(TestSelect, Subtest_1)
{
    memdb::Database db;
    db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");

    for (int i = 0; i < 100; i++)
    {
        std::string query = "INSERT (login = \"" + random_string(15) + "\", password_hash = 0x" + random_string(10) + ") to first";
        db.execute(query);
    }
    
    auto s1 = db.execute("SELECT id / 2 + id / 4 * 2 from first WHERE 1 = 1");
    auto s2 = db.execute("SELECT id / 2 + id / 4 * 2 from first WHERE |login| / 3 = 5 && !(id % 2 = 1) && login < \"TEST\"");

    // for (auto& row: s2)
    // {
    //     std::cout << row.get<int>("1") << "\n";
    // }
    ASSERT_TRUE(s1.is_ok());
    ASSERT_TRUE(s2.is_ok());
}


TEST(TestUpdate, Subtest_1)
{
    memdb::Database db;
    db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");

    for (int i = 0; i < 100; i++)
    {
        std::string query = "INSERT (login = \"" + random_string(15) + "\", password_hash = 0x" + random_string(10) + ") to first";
        db.execute(query);
    }
    
    auto s1 = db.execute("UPDATE first SET id = id / 2 + id / 4 * 2 WHERE true");
    auto s2 = db.execute("UPDATE first SET id = id + |login| + |password_hash|, login = login + \"_updated\" WHERE |login| / 3 = 5 && !(id % 2 = 1) && login < \"TEST\"");

    // for (auto& row: s2)
    // {
    //     std::cout << row.get<int>("1") << "\n";
    // }
    ASSERT_TRUE(s1.is_ok());
    ASSERT_TRUE(s2.is_ok());
}


TEST(TestDelete, Subtest_1)
{
    memdb::Database db;
    db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");

    for (int i = 0; i < 100; i++)
    {
        std::string query = "INSERT (login = \"" + random_string(15) + "\", password_hash = 0x" + random_string(10) + ") to first";
        db.execute(query);
    }
    
    auto s1 = db.execute("DELETE first WHERE |login| / 3 = 5 && !(id % 2 = 1) && login < \"TEST\"");
    auto s2 = db.execute("DELETE first WHERE 1 = 1");

    ASSERT_TRUE(s1.is_ok());
    ASSERT_TRUE(s2.is_ok());
}


TEST(TestSerial, Subtest_1)
{
    memdb::Database db, db1;
    db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");

    for (int i = 0; i < 100; i++)
    {
        std::string query = "INSERT (login = \"" + random_string(15) + "\", password_hash = 0x" + random_string(10) + ") to first";
        db.execute(query);
    }
    db.save_to_file(std::ofstream("db.bin", std::ios::binary));

    db1.load_from_file(std::ifstream("db.bin", std::ios::binary));

    auto a = db.get("first");
    auto b = db1.get("first");

    auto c = a.begin();
    auto d = b.begin();

    ASSERT_TRUE(a.rows == 100);

    for (int i = 0; i < a.rows; i++)
    {
        ASSERT_TRUE(c.get<int>("id") == d.get<int>("id"));
        ASSERT_TRUE(c.get<std::string>("login") == d.get<std::string>("login"));
        ASSERT_TRUE(c.get<bool>("is_admin") == d.get<bool>("is_admin"));
    }

}




int main(int argc, char **argv)
{
::testing::InitGoogleTest(&argc, argv);

return RUN_ALL_TESTS();
}
