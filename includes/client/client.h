#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <util.h>
#include <server/command.h>

namespace rds
{
    class Client
    {
    private:
        const std::string server_ip_;
        short server_port_;
        std::string recv_buffer_;

    public:
        void InputCommand(const std::string &);
        void Wait();
    };

} // namespace rds

#endif