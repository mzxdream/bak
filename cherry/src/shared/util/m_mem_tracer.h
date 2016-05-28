#ifndef _M_MEM_TRACER_H_
#define _M_MEM_TRACER_H_

#include <mutex>
#include <malloc.h>
#include <map>
#include <string>
#include <iostream>

template<typename T>
struct MMemTracerAllocator
    :std::allocator<T>
{
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef MMemTracerAllocator<U> other;
    };

    MMemTracerAllocator(){}

    template<typename U>
    MMemTracerAllocator(MMemTracerAllocator<U> const &u)
        :std::allocator<T>(u)
    {
    }

    pointer allocate(size_type size, std::allocator<void>::const_pointer = 0)
    {
        void *p = ::malloc(size * sizeof(T));
        if (p == 0)
        {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(p);
    }
    
    void deallocate(pointer p, size_type)
    {
        ::free(p);
    }
};

struct MMemTracerInfo
{
    const char *p_file;
    int line;
    size_t size;
};

class MMemTracerExit;

class MMemTracer
{
private:
    MMemTracer(const MMemTracer &);
    MMemTracer& operator=(const MMemTracer &);
public:
    static void Add(void *p_memory, const char *p_file, int line, size_t size)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        MMemTracerInfo info;
        info.p_file = p_file;
        info.line = line;
        info.size = size;
        memory_map_[p_memory] = info;
    }
    static void Remove(void *p_memory)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        std::map<void*, MMemTracerInfo, std::less<void*>, MMemTracerAllocator<std::pair<const char *, MMemTracerInfo> > >::iterator it 
            = memory_map_.find(p_memory);
        if (it != memory_map_.end())
        {
            memory_map_.erase(it);
        }
    }
    static void Dump()
    {
        
        std::map<void*, MMemTracerInfo, std::less<void*>, MMemTracerAllocator<std::pair<const char *, MMemTracerInfo> > >::iterator it 
            = memory_map_.begin();
        for (; it != memory_map_.end(); ++it)
        {
            std::cerr << "memory leaks at file:" << it->second.p_file << " line:" << it->second.line << " size:" << it->second.size << std::endl;
        }
    }
private:
    static std::map<void*, MMemTracerInfo, std::less<void*>, MMemTracerAllocator<std::pair<const char *, MMemTracerInfo> > > memory_map_;
    static std::mutex mtx_;
    static MMemTracerExit exit_;
};

class MMemTracerExit
{
public:
    ~MMemTracerExit()
    {
        MMemTracer::Dump();
    }
};

std::map<void*, MMemTracerInfo, std::less<void*>, MMemTracerAllocator<std::pair<const char *, MMemTracerInfo> > > MMemTracer::memory_map_;
std::mutex MMemTracer::mtx_;
MMemTracerExit MMemTracer::exit_;

inline void* operator new(size_t size, const char *p_file, int line)
{
    void *p_memory = ::malloc(size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, p_file, line, size);
    }
    return p_memory;
}

inline void operator delete(void *p_memory, const char *p_file, int line)
{
    if (p_memory)
    {
        MMemTracer::Remove(p_memory);
        ::free(p_memory);
    }
}

inline void* operator new(size_t size)
{
    void *p_memory = ::malloc(size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, "?", 0, size);
    }
    return p_memory;
}

inline void operator delete(void *p_memory)
{
    if (p_memory)
    {
        MMemTracer::Remove(p_memory);
        ::free(p_memory);
    }
}

inline void* operator new[](size_t size, const char *p_file, int line)
{
    void *p_memory = ::malloc(size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, p_file, line, size);
    }
    return p_memory;
}

inline void operator delete[](void *p_memory, const char *p_file, int line)
{
    if (p_memory)
    {
        MMemTracer::Remove(p_memory);
        ::free(p_memory);
    }
}

inline void* operator new[](size_t size)
{
    void *p_memory = ::malloc(size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, "?", 0, size);
    }
    return p_memory;
}

inline void operator delete[](void *p_memory)
{
    if (p_memory)
    {
        MMemTracer::Remove(p_memory);
        ::free(p_memory);
    }
}

inline void* _malloc(size_t size, const char *p_file, int line)
{
    void *p_memory = ::malloc(size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, p_file, line, size);
    }
    return p_memory;
}

inline void* _calloc(size_t n, size_t size, const char *p_file, int line)
{
    void *p_memory = ::calloc(n, size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, p_file, line, size);
    }
    return p_memory;
}

inline void* _realloc(void *p_memory, size_t size, const char *p_file, int line)
{
    if (p_memory)
    {
        MMemTracer::Remove(p_memory);
    }
    p_memory = ::realloc(p_memory, size);
    if (p_memory)
    {
        MMemTracer::Add(p_memory, p_file, line, size);
    }
    return p_memory;
}

inline void _free(void *p_memory)
{
    if (p_memory)
    {
        MMemTracer::Remove(p_memory);
        ::free(p_memory);
    }
}

#define new            new(__FILE__, __LINE__)
#define malloc(s)      _malloc(s, __FILE__, __LINE__)
#define calloc(n, s)   _calloc(n, s, __FILE__, __LINE__)
#define realloc(p, s)  _realloc(p, s, __FILE__, __LINE__)
#define free(p)        _free(p)

#endif 
