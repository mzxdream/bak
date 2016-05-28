#include <util/m_circle_buffer.h>
#include <cstring>

MCircleBuffer::MCircleBuffer(size_t len)
    :len_(len+1)
{
    p_buf_ = new char[len_];
    p_start_ = p_buf_;
    p_end_ = p_buf_;
}

MCircleBuffer::~MCircleBuffer()
{
    delete[] p_buf_;
}

bool MCircleBuffer::Peek(void *p_buf, size_t len)
{
    if (p_end_ >= p_start_)
    {
        if (len > static_cast<size_t>(p_end_-p_start_))
        {
            return false;
        }
        memcpy(p_buf, p_start_, len);
        p_start_ += len;
        return true;
    }
    else
    {
        if (len < (len_ - static_cast<size_t>(p_start_-p_buf_)))
        {
            memcpy(p_buf, p_start_, len);
            p_start_ += len;
            return true;
        }
        else if (len <= (len_ - static_cast<size_t>(p_start_-p_end_)))
        {
            size_t end_len = len_ - static_cast<size_t>(p_start_-p_buf_);
            memcpy(p_buf, p_start_, end_len);
            p_start_ += len;
            p_start_ -= len_;
            if (p_start_ > p_buf_)
            {
                memcpy(static_cast<char*>(p_buf)+end_len, p_buf_, p_start_-p_buf_);
            }
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool MCircleBuffer::Append(const char *p_buf, size_t len)
{
    if (p_end_ >= p_start_)
    {
        if (len >= len_ - static_cast<size_t>(p_end_-p_start_))
        {
            return false;
        }
        if (len < len_ - static_cast<size_t>(p_end_-p_buf_))
        {
            memcpy(p_end_, p_buf, len);
            p_end_ += len;
            return true;
        }
        else
        {
            size_t end_len = len_ - static_cast<size_t>(p_end_-p_buf_);
            memcpy(p_end_, p_buf, end_len);
            p_end_ += len;
            p_end_ -= len_;
            if (p_end_ > p_buf_)
            {
                memcpy(p_buf_, p_buf+end_len, p_end_-p_buf_);
            }
            return true;
        }
    }
    else
    {
        if (len >= static_cast<size_t>(p_start_-p_end_))
        {
            return false;
        }
        memcpy(p_end_, p_buf, len);
        p_end_ += len;
        return true;
    }
}

std::pair<char*, size_t> MCircleBuffer::GetNextCapacity()
{
    if (p_end_ >= p_start_)
    {
        size_t len = len_ - static_cast<size_t>(p_end_-p_buf_);
        if (p_start_ == p_buf_)
        {
            --len;
        }
        return std::make_pair(p_end_, len);
    }
    else
    {
        size_t len = static_cast<size_t>(p_start_-p_end_) - 1;
        return std::make_pair(p_end_, len);
    }
}

bool MCircleBuffer::AddEndLen(size_t len)
{
    if (p_end_ >= p_start_)
    {
        if (len >= len_ - static_cast<size_t>(p_end_-p_start_))
        {
            return false;
        }
        p_end_ += len;
        if (p_end_ >= p_buf_+len_)
        {
            p_end_ -= len_;
        }
    }
    else
    {
        if (len >= static_cast<size_t>(p_start_-p_end_))
        {
            return false;
        }
        p_end_ += len;
    }
    return true;
}

std::pair<const char*, size_t> MCircleBuffer::GetNextData()
{
    if (p_end_ >= p_start_)
    {
        return std::pair<const char*, size_t>(p_start_, p_end_-p_start_);
    }
    else
    {
        return std::pair<const char*, size_t>(p_start_, len_ - static_cast<size_t>(p_start_-p_buf_));
    }
}

bool MCircleBuffer::AddStartLen(size_t len)
{
    if (p_end_ >= p_start_)
    {
        if (len > static_cast<size_t>(p_end_-p_start_))
        {
            return false;
        }
        p_start_ += len;
    }
    else
    {
        if (len > (len_ - static_cast<size_t>(p_start_-p_end_)))
        {
            return false;
        }
        p_start_ += len;
        if (p_start_ >= p_buf_ + len_)
        {
            p_start_ -= len_;
        }
    }
    return true;
}

size_t MCircleBuffer::GetLen() const
{
    if (p_end_ >= p_start_)
    {
        return p_end_ - p_start_;
    }
    else
    {
        return len_ - static_cast<size_t>(p_start_-p_end_);
    }
}
