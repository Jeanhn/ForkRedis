#include <server/handler.h>
#include <server/loop.h>

namespace rds
{
    Handler::Handler(const RedisConf &conf) : conf_(conf) {}

    void Handler::ExecCommand(Handler *hdlr)
    {
        while (hdlr->running_)
        {
            auto cmd = hdlr->cmd_que_.BlockPop();
            auto respond = cmd->Exec();
            if (!respond.has_value())
            {
                continue;
            }
            auto client = cmd->cli_.lock();
            if (!client)
            {
                continue;
            }
            client->Append(std::move(respond.value()));
            client->EnableSend();
        }
    }

    void Handler::ExecTimer(Handler *hdlr)
    {
        while (hdlr->running_)
        {
            auto tmr = hdlr->tmr_que_.BlockPop();
            tmr->Exec();
        }
    }

    void Handler::Run()
    {
        running_ = true;
        for (int i = 0; i < std::min(conf_.cpu_num_, 3); i++)
        {
            std::thread hdl_cli_(ioClient, this);
            workers_.push_back(std::move(hdl_cli_));
        }
        std::thread exec_cmd_(ExecCommand, this);
        workers_.push_back(std::move(exec_cmd_));
        std::thread exec_tmr_(ExecTimer, this);
        workers_.push_back(std::move(exec_tmr_));
    }

    void Handler::ioClient(Handler *hdlr)
    {
        while (hdlr->running_)
        {
            std::unique_lock<std::mutex> lk(hdlr->cli_mtx_);
            hdlr->condv_.wait(lk, [&que = hdlr->cli_que_]()
                              { return !que.empty(); });
            while (!hdlr->cli_que_.empty())
            {
                auto cli_evt = hdlr->cli_que_.front();
                hdlr->cli_que_.pop();
                auto client = cli_evt.first.lock();
                if (!client)
                {
                    continue;
                }
                if (cli_evt.second == EPOLLOUT)
                {
                    int nwrite = client->Send();
                    if (nwrite == -1 && errno == EPIPE)
                    {
                        client->Logout();
                    }
                    if (client->IsSendOut() && errno != EAGAIN)
                    {
                        client->EnableRead();
                    }
                }
                else
                {
                    int nread = client->Read();
                    if (nread == 0)
                    {
                        client->Logout();
                        continue;
                    }
                    if (nread == -1 && errno == EAGAIN)
                    {
                        continue;
                    }
                    auto reqs = client->ExportMessages();
                    for (auto &req : reqs)
                    {
                        if (hdlr->conf_.enable_aof_)
                        {
                            client->GetDB()->AppendAOF(req);
                            if (hdlr->conf_.aof_mode_ == "no")
                            {
                                auto atmr = std::make_unique<AofTimer>(GetGlobalLoop().GetAOFTimer());
                                hdlr->tmr_que_.Push(std::move(atmr));
                            }
                        }
                        auto cmd = RequestToCommandExec(client, &req);
                        if (cmd)
                        {
                            hdlr->cmd_que_.Push(std::move(cmd));
                        }
                    }
                }
            }
        }
    }

    void Handler::Handle(std::pair<std::weak_ptr<ClientInfo>, int> cli_evt)
    {
        std::lock_guard<std::mutex> lg(cli_mtx_);
        cli_que_.push(std::move(cli_evt));
        condv_.notify_all();
    }

    void Handler::Handle(std::unique_ptr<Timer> timer)
    {
        tmr_que_.Push(std::move(timer));
    }
} // namespace rds
