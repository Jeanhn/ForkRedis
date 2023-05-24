#include <database/disk.h>
#include <filesystem>

namespace rds
{
    DiskManager::DiskManager(const std::string &filename) : DiskManager(filename, 0)
    {
    }

    DiskManager::~DiskManager()
    {
        Flush();
    }

    DiskManager::DiskManager(const std::string &filename, std::size_t write_cache_size) : filename_(filename),
                                                                                          write_cache_size_(write_cache_size)
    {
        file_strm_.open(filename, std::ios::app | std::ios::in | std::ios::out);
        if (std::filesystem::file_size(filename) == 0)
        {
            file_strm_ << "REDIS"
                       << "0006"
                       << "e" << std::uint64_t(0x12345678);
            file_strm_.flush();
        }
        file_strm_.close();
        file_strm_.open(filename, std::ios::app | std::ios::in | std::ios::out);
    }

    void DiskManager::Truncate()
    {
        file_strm_.close();
        file_strm_.open(filename_, std::ios::in | std::ios::out);
    }

    void DiskManager::Flush()
    {
        std::ostreambuf_iterator<char> outer(file_strm_);
        while (!write_cache_.empty())
        {
            outer = write_cache_.front();
            write_cache_.pop_front();
        }
        file_strm_.flush();
    }

    auto DiskManager::LoadAndExport() -> std::deque<char>
    {
        std::istreambuf_iterator<char> iner(file_strm_);
        while (!file_strm_.eof())
        {
            read_cache_.push_back(*iner);
        }
        std::deque<char> ret = std::move(read_cache_);
        read_cache_ = std::deque<char>{};
        return ret;
    }

    void DiskManager::Write(const std::string &data)
    {
        if (write_cache_size_ == 0)
        {
            file_strm_ << data;
            return;
        }
        std::copy(data.cbegin(), data.cend(), std::back_inserter(write_cache_));
        if (write_cache_.size() >= write_cache_size_)
        {
            Flush();
        }
    }

    void DiskManager::EnCached(std::size_t cache_size)
    {
        write_cache_size_ = cache_size;
    }
    void DiskManager::DisCached()
    {
        write_cache_size_ = 0;
    }

} // namespace rds
