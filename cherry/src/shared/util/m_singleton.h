#ifndef _M_SINGLETON_H_
#define _M_SINGLETON_H_

#include <mutex>
#include <thread/m_null_mutex.h>

template<typename TClass, typename TMutex = MNullMutex>
class MSingleton
{
protected:
    MSingleton() = default;
    ~MSingleton() = default;
    MSingleton(const MSingleton &) = delete;
    MSingleton& operator=(const MSingleton &) = delete;
public:
    static TClass& Instance()
    {
        return instance_.Instance();
    }
private:
    class MSingletonBase
    {
    public:
        MSingletonBase()
            :p_instance_(nullptr)
        {
        }
        ~MSingletonBase()
        {
            if (p_instance_)
            {
                delete p_instance_;
                p_instance_ = nullptr;
            }
        }
        MSingletonBase(const MSingletonBase &) = delete;
        MSingletonBase& operator=(const MSingleton &) = delete;
    public:
        TClass& Instance()
        {
            if (!p_instance_)
            {
                std::lock_guard<TMutex> lock(mtx_);
                if (!p_instance_)
                {
                    p_instance_ = new TClass();
                }
            }
            return *p_instance_;
        }
    private:
        TClass *p_instance_;
        TMutex mtx_;
    };
private:
    static MSingletonBase instance_;
};

template<typename TClass, typename TMutex>
typename MSingleton<TClass, TMutex>::MSingletonBase MSingleton<TClass, TMutex>::instance_;

#endif
