#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <util.h>
#include <objects/str.h>
#include <database/db.h>
#include <json11.hpp>
#include <condition_variable>
#include <queue>

namespace rds
{
    struct CommandBase;

    class ClientInfo;

    auto RawCommandToRequest(const std::string &) -> json11::Json::array;

    /* ensure that if ret-val is not null, then it can be execed  */
    auto RequestToCommandExec(std::shared_ptr<ClientInfo> client, json11::Json::array *req) -> std::unique_ptr<CommandBase>;

    struct CommandBase
    {
        bool valid_{true};
        std::string command_;
        std::string obj_name_;
        std::shared_ptr<Object> obj_;
        std::weak_ptr<ClientInfo> cli_;
        virtual auto Exec() -> std::optional<json11::Json::array> = 0;
        CLASS_DEFAULT_DECLARE(CommandBase);
    };

    class Handler;
    struct CliCommand : CommandBase
    {
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(CliCommand);
    };

    struct DbCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(DbCommand);
    };

    struct StrCommand : CommandBase
    {
        std::optional<std::string> value_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(StrCommand);
    };

    struct ListCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(ListCommand);
    };

    struct HashCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(HashCommand);
    };

    struct SetCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(SetCommand);
    };

    struct ZSetCommand : CommandBase
    {
        std::vector<Str> values_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(ZSetCommand);
    };

    struct ServerCommand : CommandBase
    {
        std::string ip_;
        short port_;
        auto Exec() -> std::optional<json11::Json::array> override;
        CLASS_DEFAULT_DECLARE(ServerCommand);
    };

    class CommandQue
    {
    private:
        /* data */
        std::mutex mtx_;
        std::condition_variable condv_;
        std::queue<std::unique_ptr<CommandBase>> que_;

    public:
        void Push(std::unique_ptr<CommandBase> cmd)
        {
            std::lock_guard<std::mutex> lg(mtx_);
            que_.push(std::move(cmd));
            condv_.notify_all();
        }
        auto BlockPop() -> std::unique_ptr<CommandBase>
        {
            std::unique_lock<std::mutex> ul(mtx_);
            condv_.wait(ul, [&que = que_]()
                        { return !que.empty(); });
            auto ret = std::move(que_.front());
            que_.pop();
            return ret;
        }
        CommandQue(/* args */) = default;
        ~CommandQue() = default;
        CLASS_DECLARE_uncopyable(CommandQue);
    };
} // namespace rds

#endif