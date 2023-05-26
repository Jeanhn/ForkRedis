# rds



to add: 
    ## RedisStrToInt done.
    string's integer handle(stoi can't handle int under zero)

to mod:
    List.Rem
    Set.RandMember

to test:
    1. all command of object
    2. command of db
    3. command of cli
    4. timer

to rebuild:
    1. the server/*
    2. command
    3. timer

BUG:
    list.rem return 0 (stoi?)
    zset. almost all (add some cout to check)