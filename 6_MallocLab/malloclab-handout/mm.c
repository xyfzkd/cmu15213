/*
 mm.c Implicit List links all blocks
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "malloclab",
    /* First member's full name */
    "Yufeng Xie",
    /* First member's email address */
    "xyf19@mails.tsinghua.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* basic constants and macros */
#define WSIZE 4 /* word_t size in header or footer area */
#define DSIZE 8
#define CHUNKSIZE (1<<3)

/* rounds up to the nearest multiple of DSIZE */
#define align(size) (((size) + (DSIZE-1)) & ~0x7)

#define get(p) (*(unsigned int *)(p))
/* get the size of the block from header element */
#define get_size(p) (get(p) & ~0x7)
/* get the alloc of the block from header element */
#define get_alloc(p) (get(p) & 0x1)
/* merge the size and alloc information */
#define pack(size, alloc) ((size)|(alloc))
/* put pack into block */
#define put(p,val) (*(unsigned int *)(p)=(val))

#define max(x, y) ((x)>(y)?(x):(y))

/* bp point to the payload of the block, h_bp calculate the header address
 * while f_bp calculate the footer address
 * and there are next_block and prev_block to locate the address of 
 * corresponding block
 */
#define h_bp(bp) ((char *)(bp) - WSIZE)
#define f_bp(bp) ((char *)(bp) + get_size(h_bp(bp))-DSIZE)
#define next_block(bp) ((char *)(bp) + get_size(((char *)(bp)-WSIZE)))
#define prev_block(bp) ((char *)(bp) - get_size(((char *)(bp)-DSIZE)))

static char* heap_lists;
static char* heap_liste;

/******************************************************************
 * heap consistency checker
 * Is every block in the free list marked as free?
 * Are there any contiguous free blocks that somehow escaped coalescing?
 * Is every free block actually in the free list?
 * Do the pointers in the free list point to valid free blocks?
 * Do any allocated blocks overlap?
 * Do the pointers in a heap block point to valid heap addresses?
 ******************************************************************/

void *escaped_coalescing();


/******************************************************************
 * Implicit List: Function prototypes
 *****************************************************************/
static void *coalesce(void *bp);
void *find_fit(size_t asize);
static void *extend_heap(size_t words);
int mm_init(void);
void place(char *bp, size_t asize);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);



/******************************************************************
 * Implicit List: Routines
 *****************************************************************/

// first fit
void *find_fit(size_t asize){
    char *bp=(char*) mem_heap_lo()+4*WSIZE,*hbp;
    for(; bp!=(char*) mem_heap_hi()+1; bp = next_block(bp)){
        hbp = h_bp(bp);
        if (!(get_alloc(hbp)) && (asize<=get_size(hbp)))
            return bp;
    }
    return NULL;
}
// next fit
int first = 1, split=0;
char *next_bp;
void *next_fit(size_t asize){
    char *hbp = h_bp(next_bp);
    if (!(get_alloc(hbp)) && (asize<=get_size(hbp))){
        return next_bp;
    }
    return find_fit(asize);
}


static void *extend_heap(size_t words){
    char *bp;
    size_t size;
    size = (words%2)?(words+1)*WSIZE:words*WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    put(h_bp(bp),pack(size,0));
    put(f_bp(bp),pack(size,0));
    put(h_bp(next_block(bp)),pack(0,1));
    return next_bp=coalesce(bp);
}
/* 
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
    if ((heap_lists = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    put(heap_lists,0);
    put(heap_lists+(1*WSIZE),pack(DSIZE,1));
    put(heap_lists+(2*WSIZE),pack(DSIZE,1));
    heap_liste = heap_lists+(3*WSIZE);
    put(heap_liste,pack(0,1));
    heap_lists += 2*WSIZE;
    if (extend_heap(CHUNKSIZE/WSIZE)==NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void place(char *bp, size_t asize){
    size_t fit_size = get_size(h_bp(bp));
    if((fit_size - asize)>= 2*DSIZE){
        put(h_bp(bp), pack(asize,1));
        put(f_bp(bp), pack(asize,1));
        char *bpn = next_block(bp);
        put(h_bp(bpn), pack(fit_size-asize,0));
        put(f_bp(bpn), pack(fit_size-asize,0));
        //next fit
        split = 1;
        next_bp = bpn;
    } else{
        put(h_bp(bp), pack(fit_size,1));
        put(f_bp(bp), pack(fit_size,1));
        split = 0;
    }
}
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    //escaped_coalescing();
    if(size == 0) return NULL;
    if (size <= DSIZE) asize = 2*DSIZE;
    else asize = align(size) + DSIZE;
    if ((bp = next_fit(asize))!=NULL)
    {
        place(bp, asize);
        return bp;
    }
    extendsize = max(asize,CHUNKSIZE);
    if((bp=extend_heap(extendsize/WSIZE))==NULL) return NULL;
    place(bp,asize);

    return bp;
}
/*
 * coalasce
 */
static void *coalesce(void *bp){
    size_t prev_alloc = get_alloc(f_bp(prev_block(bp)));
    size_t next_alloc = get_alloc(h_bp(next_block(bp)));
    size_t size = get_size(h_bp(bp));

    if (prev_alloc && next_alloc) return bp;
    else if (prev_alloc && !next_alloc){
        size += get_size(h_bp(next_block(bp)));
        put(h_bp(bp),pack(size,0));
        put(f_bp(bp),pack(size,0));
    }
    else if (!prev_alloc && next_alloc){
        size += get_size(f_bp(prev_block(bp)));
        put(h_bp(prev_block(bp)),pack(size,0));
        put(f_bp(prev_block(bp)),pack(size,0));
        bp = prev_block(bp);
    }
    else{
        size += get_size(f_bp(prev_block(bp)))+get_size(h_bp(next_block(bp)));
        put(h_bp(prev_block(bp)),pack(size,0));
        put(f_bp(prev_block(bp)),pack(size,0));
        bp = prev_block(bp);
    }
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = get_size(h_bp(ptr));
    put(h_bp(ptr),pack(size,0));
    put(f_bp(ptr),pack(size,0));
    next_bp = coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (!size) {
        mm_free(ptr);
        return NULL;
    }
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    if(oldptr == NULL)
        return newptr;
    copySize = get_size(h_bp(oldptr))-DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}





/******************************************************************
 * heap consistency checker routines
 ******************************************************************/
void *escaped_coalescing(){
    char *bp = (char*)mem_heap_lo()+4*WSIZE;
    char *lastbp = (char*)mem_heap_hi()+1;
    int nfblocks = 0, nablocks=0;
    int fblock = 0;
    printf("from address %p to %p, check contiguous free block:\n",(void*)bp,(void *)lastbp);
    for(;bp!=lastbp;bp=next_block(bp)){
        if(get_alloc(h_bp(bp))==0){
            if(fblock==1){
                printf("bp pointing to %p escapes coalescing with its former block\n",(void *)bp);
                return (void *)bp;
            }
            fblock = 1;
            nfblocks++;
        }else{
            fblock =0;
            nablocks++;
        }
    }
    printf("No escaped coalescing, congras! There are %d alloc blocks and %d free blocks.\n",
            nablocks,nfblocks);
    return NULL;
}







