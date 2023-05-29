#include <rds.h>

int main()
{
    rds::RedisConf conf = rds::DefaultConf();
    rds::MainLoop loop(conf);

    rds::Log("rds has started running...");
    while (1)
    {
        loop.Run();
    }
}