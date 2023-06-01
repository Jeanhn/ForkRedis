# rds

## Surport:
### consistence:
- RDB/AOF Persistence
【RDB或AOF持久化】
- RDB file compress
【RDB文件压缩】
- AOF file rewrite
【AOF文件重写】
- Auto delete expired kv-obj when loading RDB/AOF file
【加载RDB或AOF存储文件时自动删除过期键】
### time event:
- Any time event can be triggered with no delay
【时间事件无延迟触发】
### concurrency:
- Parallel read and concurrent write
【并行读操作以及并发写操作】
### distribution:
- Data migrate to an another remote server
【数据可远程迁移至另一服务器】

---
## Configfile:redis-conf.json
***- notice: all field should be written. there is not any of them can be ignored, otherwise the default conf will be loaded instead.配置文件的每个field都不能被省略，否则会加载默认配置***
### e.g.:
{
    "aof": true,
    "aof_mode": "no"
    "compress": true,
    "cpu": 2,
    "dbfile": "dump.db",
    "ip": "127.0.0.1",
    "memsiz_mb": 4096,
    "password": "yeah",
    "port": 8080,
    "sec": 1,
    "time": 1,
}
- aof: if enable aof.
【是否开启aof】
- aof_mode: always,no,every_sec
【aof模式的三种选择】
- compress: if enable compress in rdb
【是否开启rdb文件压缩】
- cpu: io_thread num
【io线程数】
- dbfile: database file name
【db文件名】
- memsiz_mb: memory limit size. MBytes.
【内存容积】
- password: for some high priority ops.
【更高权限密码】
- ip & port
【服务器地址】
- sec & time: for rdb's save frequency. every [sec] seconds save [time] times.
【rdb存储频率】

## Commands:
### string commands:
- set [key] [value]
- get [key]
- append [key]
- incrby [key] [int-value]
- decrby [key] [int-value]
- len [key]
### list commands:
- lpushf [key] [value1] [value2] ... // list push front
- lpushb [key] [value1] [value2] ... // push back
- lpopf [key]
- lpopb [key]
- llen [key]
- lindex [key] [index] // return value at the list[index]
- ltrim [key] [index1] [index2] // remove values whose index is between index1 and index2
- lset [key] [index] [value] // list[index]=value
### set commands:
- sadd [key] [value1] [value2] ...
- scard [key] // return size of set
- sismember [key] [value] // is value a member of set?
- smembers [key] // return all set members
- srandmember [key] // return a random member
- spop [key] // pop a rand member
- srem [key] [value] // remove value from set
- sdiff [key1] [key2] // return SET_KEY1-SET_KEY2
- sinter [key1] [key2] // return SET_KEY1 and SET_KEY2 's intersection
### zset commands:
- zadd [key] [score1] [value1] [score2] [value2] ...
- zcard [key] // return size
- zrem [key] [value]
- zcount [key] [score_low] [score_high] // number of members whose scores are between [s_low,s_high]
- zincrby/zdecrby [key] [int-value] [value]
- zrange [key] [index1] [index2] // return values by order of time sequence, and between [index1] and [index2] 
- zrangebyscore [key] [score_low] [score_high] // return values by order of score, between [score_low] &[score_high]
- zrangebylex [key] [str_lex_low] [str_lex_high] // return by lex-order
- zrank [key] [value] // return the [value]'s rank (by score) in the set
- zscore [key] [value] // return value's score
### hash commands:
- hset [key] [k1] [v1] [k2] [v2] ... // add (key and value) to the hash
- hget [key] [k1] [k2] ...
- hexist [key] [k1] ... // return if the key exists
- hdel [key] [k1] ...
- hlen [key]
- hgetall [key] // get all kv-pairs
- hincrby [key] [incr-value] [k] // incr one key's int-value
- hdecrby [key] [decr-value] [k]
### database command:
- del [key] // remove a key-value in db
- expire [key] [time_period](sec) // remove a kv after n secs
- when [key] // when will the expiring kv be removed
### client manage command
- select [int-value] [password] // select db whose number is [int-value] ;
***if password is right and the db doesn't exist, then create the db***
- create // create a db
- drop [int-value] // delete a db (this command has been banned)
- show // show the db in use, and all dbs exists
- fork [ip:port] // copy data of this db to the address ip:port 

## build:
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=RELEASE
    ## a server and a client will be built

building:
    lru-memory-limit

to add: 
    destribute
    aof-rewrite

to test:

BUG:

# rds
