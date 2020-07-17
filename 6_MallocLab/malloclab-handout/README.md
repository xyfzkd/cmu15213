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


### debug
free_list bug
how to find it?

* `r -f traces/am*.rep` when alloc at 694 times, corrupt!

```
692 times: from address 0xf699e020 to 0xf6af4680, check contiguous free block:
No escaped coalescing, congras! There are 420 alloc blocks and 25 free blocks.
693 times: from address 0xf699e020 to 0xf6af4680, check contiguous free block:
No escaped coalescing, congras! There are 421 alloc blocks and 25 free blocks.
``` 
* write `free_block_coont` and `free_list_count` function, set bp at `mm_malloc` and call them in `gdb`. Both of them return 1; when `c 694`, only `free_list_count` returns.

```
(gdb) call check_list()
0xf6a540e8      0xf6a57258      0xf6a857e0      80
0xf6a57258      0xf6a5d058      0xf6a540e8      3664
0xf6a5d058      0xb5b5b5b5      0xb5b5b5b5      4080

Program received signal SIGSEGV, Segmentation fault.

```
check malloc trace in .rep,

```
a 686 160
f 685
a 687 4072
a 688 4072
a 689 4072
a 690 4072
a 691 4072
a 692 4072
a 693 4072
a 694 4072
a 695 4072
```
a lot of alloc, maybe placing uncorrectly leads to the issue

```
c 690
(gdb) call check_list()
0xf6aac0e8      0xf6aaf258      0xf6add7e0      80
0xf6aaf258      0xf6ab2088      0xf6aac0e8      3664
0xf6ab2088      0xf6ab5058      0xf6aaf258      40880
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
0xf6ab5058      0xf6ab3078      0xf6ab2088      28640
0xf6ab3078      0xf6ab5058      0xf6ab2088      36800
```
one princple: be wrong at first, or be right
so check mm_free

```
break mm_free
r -f traces/amptjp-bal.rep
call check_list()
c
call check_list()
c
call check_list()
c
...
```

```
343 times: from address 0xf6964020 to 0xf69fea20, check contiguous free block:
No escaped coalescing, congras! There are 342 alloc blocks and 1 free blocks.
344 times: from address 0xf6964020 to 0xf69feac8, check contiguous free block:
No escaped coalescing, congras! There are 343 alloc blocks and 1 free blocks.

Breakpoint 1, mm_free (ptr=0xf69feab8) at mm.c:336
336     {
0xf69feb38      0xf69feb38      0xf69feb38      16

Breakpoint 1, mm_free (ptr=0xf69fe9c0) at mm.c:336
336     {
0xf69feab8      0xf69feab8      0xf69feab8      144
345 times: from address 0xf6964020 to 0xf69feb48, check contiguous free block:
No escaped coalescing, congras! There are 342 alloc blocks and 2 free blocks.
346 times: from address 0xf6964020 to 0xf69ffb38, check contiguous free block:
No escaped coalescing, congras! There are 343 alloc blocks and 2 free blocks.
347 times: from address 0xf6964020 to 0xf6a00b28, check contiguous free block:
No escaped coalescing, congras! There are 344 alloc blocks and 2 free blocks.
348 times: from address 0xf6964020 to 0xf6a00b28, check contiguous free block:
No escaped coalescing, congras! There are 345 alloc blocks and 2 free blocks.

Breakpoint 1, mm_free (ptr=0xf6a00a98) at mm.c:336
336     {
0xf6a00b90      0xf6a00b90      0xf6a00b90      64
0xf6a00b90      0xf6a00b90      0xf6a00b90      64
349 times: from address 0xf6964020 to 0xf6a00bd0, check contiguous free block:
No escaped coalescing, congras! There are 345 alloc blocks and 3 free blocks.

```
```
a 343 160
a 344 120
f 344
f 342
a 345 4072
a 346 4072
a 347 72
a 348 160
f 347
a 349 72
```

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


