#include <util.h>
namespace rds
{
    auto UsTime(void) -> std::size_t
    {
        struct timeval tv;
        std::size_t ust;

        gettimeofday(&tv, NULL);
        ust = ((std::size_t)tv.tv_sec) * 1000000;
        ust += tv.tv_usec;
        return ust;
    }

    auto MsTime(void) -> std::size_t
    {
        return UsTime() / 1000;
    }

    auto PeekInt(std::deque<char> &source) -> int
    {
        int ret;
        char *p = reinterpret_cast<char *>(&ret);
        for (std::size_t i = 0; i < sizeof(int); i++)
        {
            *p = source[i];
            p++;
        }
        source.erase(source.cbegin(), source.cbegin() + sizeof(int));
        return ret;
    }

    auto PeekSize(std::deque<char> &source) -> std::size_t
    {
        std::size_t ret;
        char *p = reinterpret_cast<char *>(&ret);
        for (std::size_t i = 0; i < sizeof(std::size_t); i++)
        {
            *p = source[i];
            p++;
        }
        source.erase(source.cbegin(), source.cbegin() + sizeof(std::size_t));
        return ret;
    }

    auto PeekString(std::deque<char> &source, std::size_t size) -> std::string
    {
        std::string ret;
        ret.append(source.cbegin(), source.cbegin() + size);
        source.erase(source.cbegin(), source.cbegin() + size);
        return ret;
    }
}