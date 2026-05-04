## P core vs E core

❯ gcc main.c && time ./a.out 0
sum: 1913920512

________________________________________________________
Executed in    8.62 secs    fish           external
   usr time    8.60 secs  530.00 micros    8.60 secs
   sys time    0.00 secs   97.00 micros    0.00 secs


~/git/vids-content/sched-affinity master* 8s
❯ gcc main.c && time ./a.out 14
sum: 1913920512

________________________________________________________
Executed in   42.73 secs    fish           external
   usr time   42.14 secs    0.16 millis   42.14 secs
   sys time    0.26 secs    1.02 millis    0.26 secs


## Cpu reservation

```
vagrant ssh
echo 1 2 3 1 2 3 | xargs -P 6 -t -n 1 time ./a.out
```
