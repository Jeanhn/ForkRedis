#include <rds.h>
#include <signal.h>

void SIGPIPE_Handler(int) {}

inline void SystemInit()
{
    signal(SIGPIPE, SIGPIPE_Handler);
}

int main()
{
    SystemInit();
    rds::Log("Loading config file...");
    rds::RedisConf conf = rds::LoadConf();
    conf.Print();

    rds::MainLoop loop(conf);

    loop.CheckAOFLoad();

    rds::Log("rds has started running...");
    while (1)
    {
        loop.Run();
    }
}