#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <util.h>
#include <objects/str.h>
#include <database/db.h>
#include <json11.hpp>

namespace rds
{
    struct CommandBase;

    auto RawCommandToJson(const std::string &) -> json11::Json::array;

    auto JsonToCommandExec(const json11::Json::array &) -> std::unique_ptr<CommandBase>;

    struct CommandBase
    {
        bool valid_{true};
        std::string command_;
        std::string obj_name_;
        Object *obj_;
        virtual auto Exec(Db *) -> json11::Json::array;
    };

    struct DbCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec(Db *) -> json11::Json::array;
    };

    struct StrCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec(Db *) -> json11::Json::array;
    };

    struct ListCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec(Db *) -> json11::Json::array;
    };

    struct HashCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec(Db *) -> json11::Json::array;
    };

    struct SetCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec(Db *) -> json11::Json::array;
    };

    struct ZSetCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec(Db *) -> json11::Json::array;
    };

} // namespace rds

#endif