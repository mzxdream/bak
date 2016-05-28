#ifndef _M_CLONEABLE_H_
#define _M_CLONEABLE_H_

class MCloneable
{
public:
    virtual ~MCloneable() = default;
    virtual MCloneable* Clone() = 0;
};

#endif
