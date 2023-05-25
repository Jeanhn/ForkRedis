#include <gtest/gtest.h>
#include <database/disk.h>
#include <database/db.h>
#include <database/rdb.h>
#include <objects/list.h>
#include <objects/set.h>
#include <objects/zset.h>
#include <objects/hash.h>
#include "util4test.h"
using namespace rds;

TEST(Disk, FileManager)
{
    fm.Change("test1.db");
    std::string dbfile;
    fm.Truncate();
    dbfile = db.Save();
    fm.Write(std::move(dbfile));
    auto cache = fm.LoadAndExport();
    ASSERT_EQ(cache.size(), dbfile.size());

    Db db2;
    db2.Load(&cache);

    DbEqual(&db, &db2);
    print("DM Test done");
}

TEST(Disk, Rdb)
{
    fm.Change("test2.db");
    fm.Truncate();
    auto dbfile = rdb.Save();
    fm.Write(std::move(dbfile));

    auto cache = fm.LoadAndExport();
    auto dbs = Rdb::Load(&cache);
    ASSERT_EQ(dbs.size(), 1);

    DbEqual(dbs.begin()->get(), &db);
    print("RDB Test done");
}