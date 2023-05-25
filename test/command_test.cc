#include <gtest/gtest.h>
#include <server/command.h>
#include <server/server.h>
#include <list>
using std::cout;
using std::endl;

using suit_t = std::vector<std::string>;

#define for_suit(suit_name) for (std::size_t i = 0; i < (suit_name).size(); i++)

rds::Db database;
rds::ClientInfo client{-1};

void init()
{
    client.database_ = &database;
}

suit_t str_cli_commands{
    "  SET  str1 100",
    "GET str1  ",
    "  INCRBY str1 -10  ",
    "GET   str1",
    " APPEND str1 00",
    "LEN  str1",
    " GET str1"};
suit_t str_commands{"SET", "GET", "LEN", "APPEND", "LEN", "GET", "INCRBY", "DECRBY"};
suit_t str_values{"100", "00", "str1", "10000"};

TEST(Command, RawCommandToRequest)
{
    for (std::size_t i = 0; i < str_cli_commands.size(); i++)
    {
        auto &raw = str_cli_commands[i];
        auto req = rds::RawCommandToRequest(raw);
        auto cmd = std::find(str_commands.cbegin(), str_commands.cend(), req[0].string_value());
        auto reqcmd = req[0].string_value();
        ASSERT_NE(cmd, str_commands.cend())
            << req[0].string_value();
        auto objname = std::find(str_values.cbegin(), str_values.cend(), req[1].string_value());
        auto reqname = req[1].string_value();
        ASSERT_NE(objname, str_commands.cend());
        ASSERT_EQ(*objname, "str1") << endl
                                    << "i:" << i << endl
                                    << "reqname:" << reqname;
    }
}

TEST(Command, Str)
{
    init();
    int i = 0;
    for (auto cmd : str_cli_commands)
    {
        auto req = rds::RawCommandToRequest(cmd);
        auto exec = rds::RequestToCommandExec(&client, req);
        if (!exec)
        {
            assert(0 && ("exec null"));
        }
        auto ret = exec->Exec();
        cout << ret[0].string_value() << endl;
        i++;
    }
    auto s = database.Get(std::string("str1"));
    cout << reinterpret_cast<rds::Str *>(s)->GetRaw() << endl;
}

suit_t list_cli_commands;
suit_t list_commands{"LPUSHF", "LPOPF", "LPUSHB", "LPOPB", "LLEN", "LTRIM", "LINDEX", "LREM", "LTRIM"};
