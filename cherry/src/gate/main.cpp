#include <signal.h>
#include <gate.h>

static bool sg_run = true;

void OnSignal(int s)
{
    switch (s)
    {
    case SIGINT:
    case SIGTERM:
        sg_run = false;
        break;
    default:
        break;
    }
}

void HookSignals()
{
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
}

void UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
}

int main(int argc, char **argv)
{
    HookSignals();
    if (!Gate::Instance().Init("ip=127.0.0.1;port = 3306;user=root;pwd=123;db=self;charset=utf8"))
    {
        return 0;
    }
    while (sg_run && Gate::Instance().Update())
    {
    }
    Gate::Instance().Clear();
    UnhookSignals();
    return 0;
}
