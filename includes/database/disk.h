#ifndef __DISK_H__
#define __DISK_H__

#include <deque>
#include <fstream>
#include <string>
#include <configure.h>

namespace rds
{
    class FileManager
    {
    private:
        std::string filename_;

    public:
        void Write(const std::string &bytes);
        void Write(std::string &&bytes);
        auto LoadAndExport() -> std::deque<char>;
        void Truncate();
        auto Name() const -> std::string;
        auto Size() const -> std::size_t;
        void Change(const std::string &filename);
        FileManager(const std::string &filename = "dump.db");
        ~FileManager();
    };

} // namespace rds

#endif