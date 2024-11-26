#include "./googletest/googletest/include/gtest/gtest.h"
#include "include/database.h"
#include <string>

using namespace std;

TEST(TestCreateTable, Subtest_1)
{
    memdb::Database db;
    auto r1 = db.execute("CREATE TABLE first ({key, autoincrement} id : int32 = 6, {unique} login: string[32] = \"defaultstring\", password_hash: bytes[8], is_admin: bool = false)");
    auto r2 = db.execute("CREATE TABLE second ({autoincrement} pos : int32 = 0, {key} id: int32, {unique} name: string[16] = \"amogus\")");
    auto r3 = db.execute("create table third ({unique} is_true: bool = false, is_false: bool = true)");
}

TEST(TestGroupName, Subtest_2)
{
    ASSERT_FALSE('b' == 'b');
    cout << "continue test after failure" << endl;
}

int main(int argc, char **argv)
{
::testing::InitGoogleTest(&argc, argv);

return RUN_ALL_TESTS();
}
