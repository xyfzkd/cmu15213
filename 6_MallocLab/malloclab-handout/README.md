# Notes on malloclab

malloclab-handout direction contains files about malloc code, grading code and trace file.

What a mess! Even though I read carefully about detailed introduction in malloc.pdf

just have a try

```bash
make
./mdrive -f short***
``` 
and just edit mm.c
## Problem
### config
```bash
make 
.rep file doesn't exist
```
edit path in config.h
### debug
finishing coding,

```bash
make
./mdrive -f some.rep
``` 

run out of memory!!!

```bash
gdb -tui -x breakpoint
``` 
check mem\_start\_brk and mem\_brk pointers,

this helps me

modifing macro correctly, don't forget ()

find\_fit for statement end sentences. avoid endless loop

place function trap one: don't leave labling free block

when I change CHUNKSIZE variable to test, some trace turn out to be invalid, -V tell us run out of memory!!!
I know MAXHEAP equals to be 20MB from mdrive.c and I check binary-bal.rep with the help of 
escaped\_coalescing function, which is inserted to mm_malloc. It tells me how many alloc block there are. As I gdb setting breakpoint at mm_malloc, the print info isn't what I want,
then check address, finding the tag always be odds. I just mislabel in place function.

### next\_fit
really relly important thing is "don't forget coalesce"

so add bp label to place, extend_heap, and free function

at first I add split and first option to distinguish the situation.fi when I do the formmer thing, this can be omitted.


## Results
first fit

```bash
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.008010   711
 1       yes  100%    5848  0.006058   965
 2       yes   99%    6648  0.010221   650
 3       yes  100%    5380  0.007679   701
 4       yes   99%   14400  0.000125115385
 5       yes   55%   12000  0.143164    84
 6       yes   51%   24000  0.255145    94
 7       yes   92%    4800  0.006289   763
 8       yes   92%    4800  0.005724   839
 9       yes   30%   14401  0.065296   221
10       yes   45%   14401  0.002246  6411
Total          78%  112372  0.509956   220

Perf index = 47 (util) + 15 (thru) = 62/100
```
next fit

```bash
trace  valid  util     ops      secs  Kops
 0       yes   91%    5694  0.003929  1449
 1       yes   93%    5848  0.002834  2064
 2       yes   97%    6648  0.005526  1203
 3       yes   98%    5380  0.004759  1130
 4       yes   99%   14400  0.000118121622
 5       yes   55%   12000  0.137656    87
 6       yes   51%   24000  0.248139    97
 7       yes   90%    4800  0.004367  1099
 8       yes   88%    4800  0.004092  1173
 9       yes   26%   14401  0.056231   256
10       yes   45%   14401  0.002107  6835
Total          76%  112372  0.469758   239


```


