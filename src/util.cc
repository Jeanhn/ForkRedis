#include <util.h>
#include <lzfse.h>
#include <cstdlib>

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

    auto PeekInt(std::deque<char> *source) -> int
    {
        int ret;
        char *p = reinterpret_cast<char *>(&ret);
        for (std::size_t i = 0; i < sizeof(int); i++)
        {
            *p = source->at(i);
            p++;
        }
        source->erase(source->cbegin(), source->cbegin() + sizeof(int));
        return ret;
    }

    auto PeekSize(std::deque<char> *source) -> std::size_t
    {
        std::size_t ret;
        char *p = reinterpret_cast<char *>(&ret);
        for (std::size_t i = 0; i < sizeof(std::size_t); i++)
        {
            *p = source->at(i);
            p++;
        }
        source->erase(source->cbegin(), source->cbegin() + sizeof(std::size_t));
        return ret;
    }

    auto PeekString(std::deque<char> *source, std::size_t size) -> std::string
    {
        std::string ret;
        ret.append(source->cbegin(), source->cbegin() + size);
        source->erase(source->cbegin(), source->cbegin() + size);
        return ret;
    }

    auto Compress(const std::string &data) -> std::string
    {
        uint8_t *src = reinterpret_cast<uint8_t *>(const_cast<char *>(data.data()));
        uint8_t *dst = reinterpret_cast<uint8_t *>(malloc(data.size()));
        std::size_t len = lzfse_encode_buffer(dst, data.size(), src, data.size(), 0);
        std::string ret;
        ret.reserve(len);
        for (std::size_t i = 0; i < len; i++)
        {
            ret.push_back(static_cast<char>(dst[i]));
        }
        free(dst);
        return ret;
    }

    auto Decompress(const std::string &data) -> std::string
    {
        uint8_t *src = reinterpret_cast<uint8_t *>(const_cast<char *>(data.data()));
        const size_t dst_size = data.size() * 2;
        uint8_t *dst = reinterpret_cast<uint8_t *>(malloc(dst_size));

        std::size_t len = 0;
        std::string ret;
        ret.reserve(dst_size);

        do
        {
            len = lzfse_decode_buffer(dst, dst_size, src, data.size(), 0);
            for (std::size_t i = 0; i < len; i++)
            {
                ret.push_back(static_cast<char>(dst[i]));
            }
        } while (len == dst_size);

        free(dst);

        ret.shrink_to_fit();
        return ret;
    }

    static std::atomic_bool __compress{false};

    auto DefineCompress() -> bool
    {
        return __compress.load();
    }

    void EnCompress()
    {
        __compress.store(true);
    }

    void DisCompress()
    {
        __compress.store(false);
    }
}