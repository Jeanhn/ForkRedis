#include <database/aof.h>
#include <server/server.h>
#include <server/loop.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
namespace rds
{
    void AOFLoad(const std::string ip, short port, std::deque<char> source)
    {
        auto rdb_str = PeekString(&source, 3);
        if (rdb_str != "AOF")
        {
            Log("AOF file loading error, return empty databases");
            return;
        }

        sockaddr_in sa;
        sa.sin_addr.s_addr = inet_addr(ip.data());
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);

        int the_svr = socket(AF_INET, SOCK_STREAM, 0);
        if (the_svr == -1)
        {
            Log("AOF Socket failed");
            return;
        }
        int ret = connect(the_svr, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
        if (ret == -1)
        {
            Log("AOF Server connect faied");
            return;
        }

        std::vector<char> buf;
        std::copy(source.cbegin(), source.cend(), std::back_inserter(buf));
        extern void SetNonBlock(int fd);

        auto beg = buf.data();
        auto end = beg + buf.size();

        while (beg != end)
        {
            int n = write(the_svr, beg, std::distance(beg, end));
            if (n == -1)
            {
                Log("AOF load failed");
                return;
            }
            beg += n;
        }

        GetGlobalLoop().AOFSyncWait();
        close(the_svr);

        return;
    }

    void AOFSave(std::vector<std::string> database_sources, FileManager *dump_file)
    {
        for (auto &db : database_sources)
        {
            dump_file->Write(db);
        }
    }

} // namespace rds
