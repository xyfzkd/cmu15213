//
// Created by 谢宇锋 on 2020/7/14.
//

/*
 mm.c Explicit List links free blocks
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
#define CHUNKSIZE (1<<4)

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

#define f_next_set(bp,val) (put(bp,(unsigned int)(val)))

#define f_prev_set(bp,val) (put(((unsigned int *)(bp)+1),(unsigned int)(val)))

#define f_next(bp) ((unsigned int *)*(unsigned int *)(bp))

#define f_prev(bp) ((unsigned int *)*((unsigned int *)(bp)+1))

/* bp point to the payload of the block, h_bp calculate the header address
 * while f_bp calculate the footer address
 * and there are next_block and prev_block to locate the address of
 * corresponding block
 */
#define h_bp(bp) ((char *)(bp) - WSIZE)
#define f_bp(bp) ((char *)(bp) + get_size(h_bp(bp))-DSIZE)
#define next_block(bp) ((char *)(bp) + get_size(((char *)(bp)-WSIZE)))
/* in the situation of No Boundary Tag, prev_block when the previous block is free*/
#define prev_block(bp) ((char *)(bp) - get_size(((char *)(bp)-DSIZE)))

static char* heap_lists;

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

unsigned int *free_list_bp;

int init = 1;



void *escaped_coalescing();


/******************************************************************
 * Explicit List: Routines
 *****************************************************************/

void *find_fit(size_t asize){
    char *hbp;
    unsigned int *bp = free_list_bp;
    if(bp==NULL) return NULL;
    do{
        hbp = h_bp(bp);
        if (!(get_alloc(hbp)) && (asize<=get_size(hbp)))
            return bp;
        bp = f_next(bp);
    }while (bp!=free_list_bp);
    return NULL;
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
    /* case 1 and case 3
     * case 1
     * E2=E1=S1=S2       E2=E1 S3=S1=S2
     *        |                 |
     *       root              root
     */
    if(free_list_bp==NULL){
        free_list_bp = (unsigned int *)bp;
        f_next_set(bp,bp);
        f_prev_set(bp,bp);
        return bp;
    }
    unsigned int *free_list_bp_old = free_list_bp;
    bp = coalesce(bp);
//    f_prev_set(bp,f_prev(free_list_bp_old));
//    f_next_set(f_prev(free_list_bp_old),bp);
    return bp;
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
    put(heap_lists+(3*WSIZE),pack(0,1));
    heap_lists += 2*WSIZE;
    size_t size = align(CHUNKSIZE);
    if ((free_list_bp = mem_sbrk(size))==(void *)-1)
        return -1;
    put(h_bp(free_list_bp),pack(size,0));
    put(f_bp(free_list_bp),pack(size,0));
    put(h_bp(next_block(free_list_bp)),pack(0,1));
    f_next_set(free_list_bp,free_list_bp);
    f_prev_set(free_list_bp,free_list_bp);
    //f_prev_set(heap_lists+2*WSIZE,free_list_bp);
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void place(char *bp, size_t asize){
    size_t fit_size = get_size(h_bp(bp));
    if((bp!=(char*)free_list_bp)) {
        if ((fit_size - asize) >= 2 * DSIZE) {
            put(h_bp(bp), pack(asize, 1));
            put(f_bp(bp), pack(asize, 1));
            char *bpn = next_block(bp);
            put(h_bp(bpn), pack(fit_size - asize, 0));
            put(f_bp(bpn), pack(fit_size - asize, 0));
            /* free remainder considered*/
            f_next_set(bpn, f_next(bp));
            f_prev_set(bpn, f_prev(bp));
            f_next_set(f_prev(bp), bpn);
            f_prev_set(f_next(bp), bpn);
        } else {
            put(h_bp(bp), pack(fit_size, 1));
            put(f_bp(bp), pack(fit_size, 1));

            f_next_set(f_prev(bp), f_next(bp));
            f_prev_set(f_next(bp), f_prev(bp));
        }
    }else if(f_next(bp)==(unsigned int *)bp){
        if ((fit_size - asize) >= 2 * DSIZE) {
            put(h_bp(bp), pack(asize, 1));
            put(f_bp(bp), pack(asize, 1));
            char *bpn = next_block(bp);
            put(h_bp(bpn), pack(fit_size - asize, 0));
            put(f_bp(bpn), pack(fit_size - asize, 0));
            free_list_bp = (unsigned int *) bpn;
            f_next_set(bpn,bpn);
            f_prev_set(bpn,bpn);
        }
        else{
            put(h_bp(bp), pack(fit_size, 1));
            put(f_bp(bp), pack(fit_size, 1));
            free_list_bp = NULL;
        }
    }else{
        if ((fit_size - asize) >= 2 * DSIZE) {
            put(h_bp(bp), pack(asize, 1));
            put(f_bp(bp), pack(asize, 1));
            char *bpn = next_block(bp);
            put(h_bp(bpn), pack(fit_size - asize, 0));
            put(f_bp(bpn), pack(fit_size - asize, 0));
            free_list_bp = (unsigned int *) bpn;
            /* free remainder considered*/
            f_next_set(bpn, f_next(bp));
            f_prev_set(bpn, f_prev(bp));
            f_next_set(f_prev(bp), bpn);
            f_prev_set(f_next(bp), bpn);
        }
        else{
            put(h_bp(bp), pack(fit_size, 1));
            put(f_bp(bp), pack(fit_size, 1));

            f_next_set(f_prev(bp), f_next(bp));
            f_prev_set(f_next(bp), f_prev(bp));
            free_list_bp = f_next(bp);
        }
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
    else asize = align(size)+DSIZE;
    if ((bp = find_fit(asize))!=NULL)
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

    if (prev_alloc && next_alloc) {
        if(free_list_bp!=NULL){
            f_next_set(f_prev(free_list_bp),bp);
            f_prev_set(bp,f_prev(free_list_bp));
            f_next_set(bp,free_list_bp);
            f_prev_set(free_list_bp,bp);

            /* root point to bp */
            free_list_bp = bp;
        } else{
            free_list_bp = bp;
            f_next_set(bp,free_list_bp);
            f_prev_set(free_list_bp,bp);
        }

        return bp;
    }
    else if (prev_alloc && !next_alloc){
        void *nf = next_block(bp);
        size += get_size(h_bp(next_block(bp)));
        put(h_bp(bp),pack(size,0));
        put(f_bp(bp),pack(size,0));

        f_next_set(f_prev(free_list_bp),bp);
        f_prev_set(bp,f_prev(free_list_bp));

        f_next_set(bp,free_list_bp);
        f_prev_set(free_list_bp,bp);

        free_list_bp = bp;
        f_next_set(f_prev(nf),f_next(nf));
        f_prev_set(f_next(nf),f_prev(nf));
    }
    else if (!prev_alloc && next_alloc){
        void *pf = prev_block(bp);
        size += get_size(f_bp(prev_block(bp)));
        put(h_bp(prev_block(bp)),pack(size,0));
        put(f_bp(prev_block(bp)),pack(size,0));
        bp = prev_block(bp);
        if(bp!=free_list_bp){
            f_prev_set(f_next(pf),f_prev(pf));
            f_next_set(f_prev(pf),f_next(pf));

            f_next_set(f_prev(free_list_bp),bp);
            f_prev_set(bp,f_prev(free_list_bp));

            f_next_set(pf,free_list_bp);
            f_prev_set(free_list_bp,pf);
        }
        free_list_bp = bp;


    }
    else{
        void *pf = prev_block(bp),*nf = next_block(bp);
        size += get_size(f_bp(prev_block(bp)))+get_size(h_bp(next_block(bp)));
        put(h_bp(prev_block(bp)),pack(size,0));
        put(f_bp(prev_block(bp)),pack(size,0));
        /* two elements in the list */
        if((f_next(nf)==pf)&&(f_next(pf)==nf)){
            f_next_set(pf,pf);
            f_prev_set(pf,pf);
            free_list_bp = pf;
            return pf;
        }
        /* three or more elements, nf and pf are adjacent, two situations */
        else if(f_next(nf)==pf){
            f_prev_set(f_next(pf),pf);
            f_next_set(pf,f_next(pf));
            f_next_set(f_prev(nf),pf);
            f_prev_set(pf,f_prev(nf));
        }else if(f_next(pf)==nf){
            f_prev_set(f_next(nf),pf);
            f_next_set(pf,f_next(nf));
            f_next_set(f_prev(pf),pf);
            f_prev_set(pf,f_prev(pf));
        }
        /* free_list_bp is neither pf nor nf */
        else if(free_list_bp!=pf&&free_list_bp!=nf){
            f_prev_set(f_next(pf),f_prev(pf));
            f_next_set(f_prev(pf),f_next(pf));
            f_next_set(f_prev(nf),f_next(nf));
            f_prev_set(f_next(nf),f_prev(nf));

            f_next_set(f_prev(free_list_bp),pf);
            f_prev_set(pf,f_prev(free_list_bp));
            f_next_set(pf,free_list_bp);
            f_prev_set(free_list_bp,pf);
        }
        /* free_list_bp is either pf or nf, now pf and nf are not adjacent, just
         * move free_list_bp to other element transform the situation be the same
         * as the formal
         */
        else{
            f_prev_set(f_next(pf),f_prev(pf));
            f_next_set(f_prev(pf),f_next(pf));
            f_next_set(f_prev(nf),f_next(nf));
            f_prev_set(f_next(nf),f_prev(nf));
            unsigned int *free_list_bp_new = f_prev(free_list_bp);
            f_next_set(f_prev(free_list_bp_new),pf);
            f_prev_set(pf,f_prev(free_list_bp_new));
            f_next_set(pf,free_list_bp_new);
            f_prev_set(free_list_bp_new,pf);
        }
        free_list_bp = pf;
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
    coalesce(ptr);
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
    static int count = 0;
    char *bp = (char*)mem_heap_lo()+4*WSIZE;
    char *lastbp = (char*)mem_heap_hi()+1;
    int nfblocks = 0, nablocks=0;
    int fblock = 0;
    printf("%d times: from address %p to %p, check contiguous free block:\n",count,(void*)bp,(void *)lastbp);
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
    count++;
    return NULL;
}

/*
 * Is every block in the free list marked as free?
 */
int mark_free(){
    unsigned int *bp = free_list_bp;
    do{
        if(get_alloc(h_bp(free_list_bp)))
            return 0;
        bp = f_next(bp);
    }while(bp!=free_list_bp);
    return 1;
}

int free_list_count(){
    unsigned int *bp = free_list_bp;
    int counts = 0;
    do{
        counts++;
        bp = f_next(bp);
    }while (bp!=free_list_bp);
    return counts;
}

int free_block_count(){
    int counts = 0;
    char *bp = (char*)mem_heap_lo()+4*WSIZE;
    char *lastbp = (char*)mem_heap_hi()+1;
    for(;bp!=lastbp;bp=next_block(bp)){
        if(!get_alloc(h_bp(bp)))
            counts++;
    }
    return counts;
}
/* print list pointer info and check list
 * a    b   c
 * b    d   a
 * d    e   b
 * check the first and second columns paired.
 * check the first and third columns paired
 */
int check_list(){
    int counts = free_block_count();
    unsigned int * bp = free_list_bp;
    unsigned int a0=0,a=0,b=0,c=0,c0=0;
    for (int i=0; i<counts; i++){
        if (i==0) {
            a0 = a = (unsigned int)bp;
            b = (unsigned int)f_next(bp);
            c0 = c = (unsigned int)f_prev(bp);
        }else {
            c = (unsigned int)f_prev(bp);
            if (a == c) a = (unsigned int)bp;
            else return 0;

            if (a == b) b = (unsigned int)f_next(bp);
            else return 0;
        }

        printf("%p\t%p\t%p\t%d\n",bp,f_next(bp),f_prev(bp),get_size(h_bp(bp)));
        bp = f_next(bp);
    }
    if ((b==a0)&&(c0==a)) return 1;
    else return 0;
}
/*
 * Is every free block actually in the free list?
 */
//int block_in_list(){
//    char *bp = (char*)mem_heap_lo()+4*WSIZE;
//    char *lastbp = (char*)mem_heap_hi()+1;
//    unsigned int *bpl = free_list_bp;
//    for(;bp!=lastbp;bp=next_block(bp)){
//        if(!get_alloc(h_bp(bp))){
//            do{
//                if(bp==bpl) break;
//                bpl = f_next(bpl);
//            }while (bpl!=free_list_bp);
//            return 0;
//        }
//    }
//    return 1;
//}