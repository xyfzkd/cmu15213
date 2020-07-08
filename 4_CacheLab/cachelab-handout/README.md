## SUMMARY 
for 32*32 case
    notice diagonal blocks for they can cause
    conflict miss when reading from A and writes
    to B

    I just use 8*8-size blocks to transpose

for 64*64 case
    They are different. The address of upper 8*8
    block are the same as the ones of lower.
    64ints*4bytes_per_ints*4lines = 2^10
    this phenomenon can be noticed by f0.v1.trace,
    the first element address in A is 0x10d0a0,
                                 B    0X14****,
    because of diagoal blocks problemed being solved,
    the upper lines in f0.v1.trace are almost hits,
    while miss follwing at 0x10d0c0, which is 
    actually the (0,1) block! Moreover, this help 
    realize the effect of initMatrix function and
    the address of A and B.

    We can see static int A[256][256] in the front
    of tracegen.c, and we set M and N, then  pass
    first address to initMatrix, what about the
    each element in A[32][32]? It'sjust continuous!
    A blank area exists betwenn A and B. 
    
    This basic C fundamental help us to notice 
    block miss in blocks.

    v2: when reading ints from A, (i,j+1) next to
    (i,j), miss doesn't exist. However, every four
    bytes reading, the writes in B lead to conflict
    miss
    I put forward a solution 
    f0.v1.trace 1731
    f0.v2.trace 1507

    A  to B
    ```
    |  .    --|
    |__|    ._|
    ```
    there are four area in the block, and we no longer
    read ins continuously, just follow the pattern.
    The dots represent the last area to read or writes.

    so when reading from A, one fourth elements cause
    conflict miss, and when writing to B, no conflict miss.

    BUT STILL PROBLEM!
