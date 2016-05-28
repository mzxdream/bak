#ifndef _M_CIRCLE_BUFFER_H_
#define _M_CIRCLE_BUFFER_H_

#include <utility>
#include <cstddef>

class MCircleBuffer
{
public:
    MCircleBuffer(size_t len);
    ~MCircleBuffer();
    MCircleBuffer(const MCircleBuffer &) = delete;
    MCircleBuffer& operator=(const MCircleBuffer &) = delete;
public:
    bool Peek(void *p_buf, size_t len);
    bool Append(const char *p_buf, size_t len);
    std::pair<char*, size_t> GetNextCapacity();
    bool AddEndLen(size_t len);
    std::pair<const char*, size_t> GetNextData();
    bool AddStartLen(size_t len);
    size_t GetLen() const;
private:
    char *p_buf_;
    size_t len_;
    char *p_start_;
    char *p_end_;
};

#endif
