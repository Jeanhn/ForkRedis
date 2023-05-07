#ifndef __DISK_H__
#define __DISK_H__

#include <deque>
#include <fstream>
#include <string>

namespace rds
{

    class DBLoader
    {
    private:
        std::fstream fs_;
        std::deque<char> cache_;
        std::string file_path_;
        void Load();

    public:
        auto Export() -> std::deque<char>;
        DBLoader(const std::string & = "rds.db");
        ~DBLoader();
    };

} // namespace rds

#endif