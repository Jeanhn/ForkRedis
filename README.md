# rds

## Surport:
### consistence:
- RDB Persistence
- Expire key-value-objects
- RDB file compress
- Auto delete expired kv-obj when loading
### time event:
- Any time event can be triggered with no delay
### concurrency:
- Parallel read and concurrent write
### distribution:
- Data migrate to another server

---

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
- select [int-value] // select db whose number is [int-value] 
- create // create a db
- drop [int-value] // delete a db (this command has been banned)
- show // show the db in use, and all dbs exists


## build:
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=RELEASE
    ## a server and a client will be built


to add: 
    aof
    lru-memory-limit

to test:

BUG:

# rds
