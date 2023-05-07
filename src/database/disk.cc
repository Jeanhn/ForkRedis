#include <database/disk.h>
#include <filesystem>

namespace rds
{
    DBLoader::DBLoader(const std::string &file_name)
    {
        file_path_ = std::filesystem::current_path().string() + file_name;
        fs_.open(file_path_,
                 std::ios::app | std::ios::in | std::ios::out);
        if (!fs_.is_open())
        {
            throw std::runtime_error("data file not open");
        }
        Load();
        std::filesystem::resize_file(file_path_, 0);
    }

    std::deque<char> DBLoader::Export()
    {
        std::deque<char> ret(std::move(cache_));
        return ret;
    }

    DBLoader::~DBLoader()
    {
    }

} // namespace rds
