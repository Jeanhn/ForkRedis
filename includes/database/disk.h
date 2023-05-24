#ifndef __DISK_H__
#define __DISK_H__

#include <deque>
#include <fstream>
#include <string>
#include <configure.h>

namespace rds
{
    class DiskManager
    {
    private:
        const std::string filename_;
        std::fstream file_strm_;
        std::deque<char> write_cache_;
        std::deque<char> read_cache_;
        std::size_t write_cache_size_{0};

    public:
        void Flush();
        void Write(const std::string &);
        auto LoadAndExport() -> std::deque<char>;
        void EnCached(std::size_t = 1024);
        void DisCached();
        void Truncate();
        DiskManager(const std::string &filename = "dump.db");
        DiskManager(const std::string &filename, std::size_t write_cache_size);
        ~DiskManager();
    };

} // namespace rds

#endif