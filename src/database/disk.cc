#include <database/disk.h>
#include <filesystem>

namespace rds
{
    FileManager::~FileManager()
    {
    }

    FileManager::FileManager(const std::string &filename) : filename_(filename)
    {
        if (!std::filesystem::exists(filename))
        {
            std::ofstream ofile_strm;
            ofile_strm.open(filename, std::ios::out);
        }
    }

    void FileManager::Truncate()
    {
        std::filesystem::resize_file(filename_, 0);
    }

    auto FileManager::LoadAndExport() -> std::deque<char>
    {
        if (std::filesystem::is_empty(filename_))
        {
            return {};
        }
        std::ifstream ifile_strm;
        ifile_strm.open(filename_, std::ios::in | std::ios::binary);

        std::deque<char> ret;
        while (!ifile_strm.eof())
        {
            char c;
            ifile_strm.read(&c, sizeof(c));
            if (!ifile_strm.eof())
            {
                ret.push_back(c);
            }
        }
        return ret;
    }

    void FileManager::Write(const std::string &bytes)
    {
        std::ofstream ofile_strm;
        ofile_strm.open(filename_, std::ios::out | std::ios::app);
        ofile_strm << bytes;
    }

    void FileManager::Write(std::string &&bytes)
    {
        std::ofstream ofile_strm;
        ofile_strm.open(filename_, std::ios::out | std::ios::app);
        ofile_strm << std::move(bytes);
    }

    auto FileManager::Name() const -> std::string
    {
        return filename_;
    }
    auto FileManager::Size() const -> std::size_t
    {
        return std::filesystem::file_size(filename_);
    }
    void FileManager::Change(const std::string &filename)
    {
        filename_ = filename;
        if (!std::filesystem::exists(filename))
        {
            std::ofstream ofile_strm;
            ofile_strm.open(filename, std::ios::out);
        }
    }

} // namespace rds
