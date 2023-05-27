#include <rds.h>

int main()
{
    rds::RedisConf conf = rds::DefaultConf();
    rds::MainLoop loop(conf);
    while (1)
    {
        loop.Run();
    }
}