#ifndef _M_ERRNO_H_
#define _M_ERRNO_H_

#include <errno.h>

enum class MError
{
    No = 0,
    Unknown = 1,
    ConnectOpened = 2,
    ConnectFailed = 3,
    Disconnect = 4,
    OutOfMemory = 5,
    NotSupport = 6,
    NoData = 7,
    NotMatch = 8,
    ConvertFailed = 9,
    Running = 10,
    Created = 11,
    InProgress = 12,
    Again = 13,
    InterruptedSysCall = 14,
    Invalid = 15,
    Underflow = 16,
    Overflow = 17,
};

#endif
