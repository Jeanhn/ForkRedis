#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <util.h>
#include <objects/str.h>
#include <database/db.h>
#include <json11.hpp>

namespace rds
{
    struct CommandBase;

    class ClientInfo;

    auto RawCommandToRequest(const std::string &) -> json11::Json::array;

    auto RequestToCommandExec(ClientInfo *client, const json11::Json::array &req) -> std::unique_ptr<CommandBase>;

    struct CommandBase
    {
        bool valid_{true};
        std::string command_;
        std::string obj_name_;
        Object *obj_;
        ClientInfo *cli_;
        virtual auto Exec() -> json11::Json::array = 0;
        CLASS_DEFAULT_DECLARE(CommandBase);
    };

    struct CliCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(CliCommand);
    };

    struct DbCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(DbCommand);
    };

    struct StrCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(StrCommand);
    };

    struct ListCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(ListCommand);
    };

    struct HashCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(HashCommand);
    };

    struct SetCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(SetCommand);
    };

    struct ZSetCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> json11::Json::array;
        CLASS_DEFAULT_DECLARE(ZSetCommand);
    };

} // namespace rds

#endif