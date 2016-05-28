#ifndef _GATE_H_
#define _GATE_H_

#include <util/m_singleton.h>
#include <functional>

class Gate
    :public MSingleton<Gate>
{
public:
    Gate();
    ~Gate();
private:
};

#endif
