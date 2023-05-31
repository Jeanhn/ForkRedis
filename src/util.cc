#include <util.h>
#include <fstream>
#include <lzfse.h>
#include <cstdlib>
#include <objects/str.h>
#include <json11.hpp>

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
        if (data.empty())
        {
            return {};
        }
        uint8_t *src = reinterpret_cast<uint8_t *>(const_cast<char *>(data.data()));
        std::vector<uint8_t> destination;

        int times = 1;
        std::size_t len = 0;
        while (len == 0)
        {
            destination.resize(data.size() * times);
            uint8_t *dst = destination.data();
            len = lzfse_encode_buffer(dst, destination.size(), src, data.size(), 0);
            times++;
        }

        std::string ret;
        ret.reserve(len);
        std::copy(destination.cbegin(), destination.cbegin() + len, std::back_inserter(ret));
        return ret;
    }

    auto Decompress(const std::string &data) -> std::string
    {
        uint8_t *src = reinterpret_cast<uint8_t *>(const_cast<char *>(data.data()));
        std::vector<uint8_t> destination;

        std::size_t len = 0;
        int times = 5;
        do
        {
            destination.resize(data.size() * times);
            uint8_t *dst = destination.data();
            len = lzfse_decode_buffer(dst, destination.size(), src, data.size(), 0);
            times++;
        } while (len == destination.size());

        std::string ret;
        std::copy(destination.cbegin(), destination.cbegin() + len, std::back_inserter(ret));
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
        Log("Enable string compress save");
    }

    void DisCompress()
    {
        __compress.store(false);
    }

    auto RedisStrToInt(const Str &value) -> std::optional<int>
    {
        auto raw = value.GetRaw();
        if (raw.empty())
        {
            return {};
        }
        if (raw[0] != '-')
        {
            for (auto c : raw)
            {
                if (c < '0' || c > '9')
                {
                    return {};
                }
            }
            return std::stoi(raw);
        }
        else
        {
            std::string abs(raw.begin() + 1, raw.end());
            if (abs.empty())
            {
                return {};
            }
            for (auto c : abs)
            {
                if (c < '0' || c > '9')
                {
                    return {};
                }
            }
            return -std::stoi(abs);
        }
    }

    auto LoadConf() -> RedisConf
    {
        std::string conf_file;
        std::ifstream ifile_strm;
        ifile_strm.open("redis-conf.json");
        if (!ifile_strm.is_open())
        {
            Log("Error:", "open redis-conf.json");
            return {};
        }
        while (!ifile_strm.eof())
        {
            char c;
            ifile_strm.read(&c, sizeof(c));
            if (!ifile_strm.eof())
            {
                conf_file.push_back(c);
            }
        }
        std::string err;
        json11::Json conf_obj = json11::Json::parse(conf_file, err);
        if (!err.empty())
        {
            Log("Error:", "parse redis-conf.json");
            return DefaultConf();
        }
        RedisConf conf;
        std::vector<std::string> fields{
            "dbfile",
            "ip",
            "port",
            "compress",
            "aof",
            "aof_mode",
            "sec",
            "time",
            "memsiz_mb",
            "cpu",
            "password"};
        auto obj_value = conf_obj.object_items();
        bool good = true;
        std::for_each(fields.cbegin(), fields.cend(), [&good, &o = obj_value](const std::string &f) mutable
                      { if (o.find(f)==o.cend()){
                Log("redis-conf.json file: ", "no", f);
                good = false;
            } });
        if (!good)
        {
            return DefaultConf();
        }
        conf.file_name_ = obj_value["dbfile"].string_value();
        conf.ip_ = obj_value["ip"].string_value();
        conf.port_ = obj_value["port"].int_value();
        conf.compress_ = obj_value["compress"].bool_value();
        conf.enable_aof_ = obj_value["aof"].bool_value();
        conf.frequence_.every_n_sec_ = obj_value["sec"].int_value();
        conf.frequence_.save_n_times_ = obj_value["time"].int_value();
        conf.mem_size_mbytes_ = obj_value["memsiz_mb"].int_value();
        conf.cpu_num_ = obj_value["cpu"].int_value();
        conf.password_ = obj_value["password"].string_value();
        conf.aof_mode_ = obj_value["aof_mode"].string_value();
        return conf;
    }

    auto DefaultConf() -> RedisConf
    {
        RedisConf conf;
        conf.file_name_ = "dump.db";
        conf.ip_ = "127.0.0.1";
        conf.port_ = 8080;
        conf.compress_ = true;
        conf.enable_aof_ = false;
        conf.frequence_.every_n_sec_ = 1;
        conf.frequence_.save_n_times_ = 1;
        conf.mem_size_mbytes_ = 4096;
        conf.cpu_num_ = 2;
        conf.password_ = "yeah";
        return conf;
    }

    static std::string password_;
    static std::mutex pswd_mtx_;
    auto SetPassword(std::string pswd) -> bool
    {
        std::lock_guard lg(pswd_mtx_);
        password_ = std::move(pswd);
        return true;
    }

    auto GetPassword() -> std::string
    {
        std::lock_guard lg(pswd_mtx_);
        return password_;
    }
}