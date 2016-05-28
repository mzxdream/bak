#ifndef _M_NONCOPYABLE_H_
#define _M_NONCOPYABLE_H_

class MNoncopyable
{
protected:
    MNoncopyable() = default;
    ~MNoncopyable() = default;
public:
    MNoncopyable(const MNoncopyable &) = delete;
    MNoncopyable& operator=(const MNoncopyable &) = delete;
};

#endif
