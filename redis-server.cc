#include <rds.h>

int main()
{
    rds::RedisConf conf;
    rds::Log("Loading config file...");
    auto c = rds::LoadConf();
    if (!c.has_value())
    {
        rds::Log("Set defualt config");
        conf = rds::DefaultConf();
    }
    else
    {
        conf = c.value();
    }
    conf.Print();

    rds::MainLoop loop(conf);

    loop.CheckAOFLoad();

    rds::Log("rds has started running...");
    while (1)
    {
        loop.Run();
    }
}