#include <objects/str.h>
#include <cstring>
#include <database/rdb.h>

namespace rds
{

    Str::Str(std::string data) : data_(data), encoding_type_(EncodingType::INT)
    {
        for (std::size_t i = 0; i < data_.size(); i++)
        {
            if (data_[i] < '0' || data_[i] > '9')
            {
                if (i == 0 && data_[i] == '-' && data_.size() > 1)
                {
                    continue;
                }
                else
                {
                    encoding_type_ = EncodingType::STR_RAW;
                    break;
                }
            }
        }
    }

    Str::Str(const Str &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_ = lhs.data_;
        encoding_type_ = lhs.encoding_type_;
    }

    Str::Str(Str &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_ = std::move(rhs.data_);
        encoding_type_ = rhs.encoding_type_;
    }

    Str &Str::operator=(const Str &lhs)
    {
        ReadGuard rg(lhs.ExposeLatch());
        data_ = lhs.data_;
        encoding_type_ = lhs.encoding_type_;
        return *this;
    }
    Str &Str::operator=(Str &&rhs) noexcept
    {
        ReadGuard rg(rhs.ExposeLatch());
        data_ = std::move(rhs.data_);
        encoding_type_ = rhs.encoding_type_;
        return *this;
    }

    auto Str::GetRaw() const -> std::string
    {
        ReadGuard rg(latch_);
        return data_;
    }

    void Str::Set(std::string data)
    {
        WriteGuard wg(latch_);
        data_ = std::move(data);
        encoding_type_ = EncodingType::INT;
        for (std::size_t i = 0; i < data_.size(); i++)
        {
            if (data_[i] < '0' || data_[i] > '9')
            {
                if (i == 0 && data_[i] == '-' && data_.size() > 1)
                {
                    continue;
                }
                else
                {
                    encoding_type_ = EncodingType::STR_RAW;
                    return;
                }
            }
        }
    }

    auto Str::Append(std::string data) -> std::size_t
    {
        WriteGuard wg(latch_);
        data_.append(std::move(data));
        if (encoding_type_ == EncodingType::INT)
        {
            auto intval = RedisStrToInt(data_);
            if (!intval.has_value())
            {
                encoding_type_ = EncodingType::STR_RAW;
            }
        }
        return data_.size();
    }

    auto Str::IncrBy(int delta) -> std::string
    {
        WriteGuard wg(latch_);
        if (encoding_type_ != EncodingType::INT)
        {
            return {};
        }
        auto intval = RedisStrToInt(data_);
        if (!intval.has_value())
        {
            encoding_type_ = EncodingType::STR_RAW;
            return {};
        }
        int raw_int = intval.value();
        raw_int += delta;
        data_ = std::to_string(raw_int);
        return data_;
    }

    auto Str::DecrBy(int delta) -> std::string
    {
        return IncrBy(-delta);
    }

    auto Str::Len() const -> std::size_t
    {
        ReadGuard rg(latch_);
        return data_.size();
    }

    auto Str::Empty() const -> bool
    {
        ReadGuard rg(latch_);
        return data_.empty();
    }

    // template <typename T, typename =
    //                           std::enable_if_t<std::is_same_v<std::string, std::decay_t<T>>, void>>
    // Str::Str(T &&data) : data_(std::forward<T>(data))
    // {
    // }

    auto Str::GetObjectType() const -> ObjectType
    {
        return ObjectType::STR;
    }

    /*
    char obj-type
    [size_t len_after_compress]/[size_t len_origin]
    [int/string]value
     */
    auto Str::EncodeValue() const -> std::string
    {
        ReadGuard rg(latch_);
        std::string ret;
        // encode-type: int or string or compress-string
        char t = EncodingTypeToChar(encoding_type_);

        assert(encoding_type_ == EncodingType::STR_RAW || encoding_type_ == EncodingType::INT || encoding_type_ == EncodingType::STR_COMPRESS);

        // if str [len] or [len len-before-compress]
        if (encoding_type_ == EncodingType::STR_RAW)
        {
            if (DefineCompress())
            {
                t = EncodingTypeToChar(EncodingType::STR_COMPRESS);
            }
            ret.push_back(t);
            std::string res;
            if (DefineCompress())
            {
                std::string cprs = Compress(data_);
                std::size_t len = cprs.size();
                assert(len != 0);
                ret.append(BitsToString(len));
                res = std::move(cprs);
            }
            else
            {
                res = data_;
            }
            std::size_t len = data_.size();
            ret.append(BitsToString(len));
            ret.append(res);
            return ret;
        }

        // int
        ret.push_back(t);
        int data = std::stoi(data_);
        ret.append(BitsToString(data));
        return ret;
    }

    auto Str::GetEncodingType() const -> EncodingType
    {
        ReadGuard rg(latch_);
        return encoding_type_;
    }

    void Str::DecodeValue(std::deque<char> *source)
    {
        WriteGuard wg(latch_);
        EncodingType etyp = CharToEncodingType(source->front());
        source->pop_front();

        assert(etyp == EncodingType::INT || etyp == EncodingType::STR_RAW || etyp == EncodingType::STR_COMPRESS);

        data_.clear();

        if (etyp == EncodingType::INT)
        {
            encoding_type_ = EncodingType::INT;
            data_ = std::to_string(PeekInt(source));
            return;
        }
        encoding_type_ = EncodingType::STR_RAW;
        if (etyp == EncodingType::STR_RAW)
        {
            size_t size = PeekSize(source);
            data_ = PeekString(source, size);
        }
        else if (etyp == EncodingType::STR_COMPRESS)
        {
            size_t size_compress = PeekSize(source);
#ifndef NDEBUG
            size_t size_origin = PeekSize(source);
#else
            PeekSize(source);
#endif
            assert(size_compress != 0);
            data_ = Decompress(PeekString(source, size_compress));
            assert(size_origin == data_.size());
        }
    }

    auto Str::Fork(const std::string &key) -> std::string
    {
        ReadGuard rg(latch_);
        return "SET " + key + " " + data_;
    }

} // namespace rds
