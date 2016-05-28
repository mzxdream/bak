#ifndef _M_TYPE_DEFINE_H_
#define _M_TYPE_DEFINE_H_

#include <stdint.h>
#include <string>

typedef pthread_t m_thread_t;

class MBlob final
{
public:
    MBlob() = default;
    ~MBlob() = default;
    MBlob(const MBlob &) = default;
    MBlob& operator=(const MBlob &) = default;
public:
    const char* GetData() const
    {
        return &data_[0];
    }
    char* GetData()
    {
        return const_cast<char*>(static_cast<const MBlob&>(*this).GetData());
    }
    size_t GetSize() const
    {
        return data_.size();
    }
    void Resize(size_t size)
    {
        data_.resize(size);
    }
private:
    std::string data_;
};

#endif
