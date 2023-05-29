# rds

## Surport:
- RDB Persistence
- Expire key-value-objects
- Auto delete expired kv-obj when loading
- Any timer can be triggered with no delay
- Concurrent read or write

## Commands:
### string commands:
- set [key] [value]
- get [key]
- append [key]
- incrby [key] [int-value]
- decrby [key] [int-value]
- len [key]
### list commands:
- lpushf [key] [value1] [value2] ... //list push front
- lpushb [key] [value1] [value2] ... //push back
- lpopf [key]
- lpopb [key]
- llen [key]
- lindex [key] [index]
- ltrim [key] [begin_index] [end_index]
- lset [key] [index] [value]
### set commands:
- sadd [key] [value1] [value2] ...
- scard [key] //return size of set
- sismember [key] [value] //is value a member of set?
- smembers [key] //return all set members
- srandmember [key] //return a random member
- spop [key] //pop a rand member
- srem [key] [value] //remove value from set
- sdiff [key1] [key2] //return SET_KEY1-SET_KEY2
- sinter [key1] [key2] //return SET_KEY1 and SET_KEY2 's intersection
### zset commands:
debugging work is undone..
### hash commands:
- hset [key] [k1] [v1] [k2] [v2] ...
- hget [key] [k1] [k2] ...
- hexist [key] [k1] ...
- hdel [key] [k1] ...
- hlen [key]
- hgetall [key]
- hincrby [key] [incr-value] [k]
- hdecrby [key] [decr-value] [k]



to add: 
    aof

to test:

BUG:
    zset. almost all (add some cout to check)

# rds
