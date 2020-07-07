#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "queue.c"
#include <getopt.h>
///************************** Data Structure declarations ***************************************************************/
///*
// * here I apply to linked list to achieve LRU cache reading, ignoring cache writing and disk wring. For cache reading, I
// * achieve O(1) time complexity. For disk writing, there are other data structure like hashmap with dual-linkedlist at
// * https://www.jianshu.com/p/b1ab4a170c3c, go ahead!
// */
///*
// * cache element, replace array with pointer.
// */
//typedef struct cache {
//    char tag;
//    char * set;
//    char * block;
//    struct cache *next;
//} cache_e;
///*
// * cache queue
// */
//typedef struct {
//    cache_e *head;
//    //cache_e *tail;
//}cache_q;
///************************** Operation on queue  ***********************************************************************/
//cache_q *q_new(); // create a new, empty queue
//void q_free(cache_q *q); // free storage by a queue
//void q_remove_head(cache_q *q); // LRU remove cache
//
///* insert element at tail of cache queue, return true if successful, return false if q is NULL or could not allocate
// * space
// */
//bool q_insert_tail(cache_q *q, char *set, char *block);

/************************** trace file repeat   ***********************************************************************/

void traceFile(char * fs, int verbose, int s , int E, int b);

int nhit=0, nmiss=0, nevict=0;

int main(int argc, char** argv)
{
    int opt,ch;
    char * tracefile;
    static int verbose=0, s, E, b;
    FILE * fp;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:"))!=-1){
        switch(opt){
            case 'h':
                fp = fopen("help_doc","r");
                if (fp == NULL){
                    printf("Failed to open file.\n");
                    exit(1);
                }
                while ((ch = getc(fp))!= EOF)
                    putchar(ch);
                fclose(fp);
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            default:
                printf("wrong argument\n");
                abort();
        }
    }
    traceFile(tracefile,verbose,s,E,b);

    printSummary(nhit, nmiss, nevict);
    return 0;
}



void hit(char ***array, unsigned long int add, char operation, int size, int s, int E, int b, int verbose, queue_t **cq);

queue_t **cacheq_new(int i, int j);

void cacheq_free(queue_t **q, int i);

void traceFile(char * fs, int verbose, int s , int E, int b){
    char operation;
    unsigned long int address;
    int size;
    FILE * pFile;
    pFile = fopen(fs,"r");
    if (pFile == NULL){
        printf("Failed to open file.\n");
        exit(1);
    }
    /* cache calloc:
     * address: tag bit (t), set index bit (s), block bit (b) ---- ttttttttssssbbbb;
     * cache: (2^s, E, m) dimension array, m = valid bit + tag bit + 2^block bit*8 bytes ---- 0ttttttttbbbbbbbbbbbbbbbbbb
     */
    int m;
    char ***array;
    //t = 64 - s - b; // 64 bits address, tag bits equals to 64-s-b;
    /* cache_size (bytes) = (pow(2,b) + ceil((t+1)/8.0)) * pow(2,s);
     * for the third dimension of cache array, m should be the former. However, to behave simply, the cache calloc more
     * space for tag bits one bytes and set index bits eight bytes. Here set index bits are ignored for this is cache
     * reading simulation. In P.437, textbook notes some terms write-back, which may add up complexity for the
     * simulation.
     * the last item is time stamp when E is not equal to 1 and LRU strategy are included.
     */
    m = 8+1+1;
    /* allocate cache */
    array=(char***)calloc(pow(2,s),sizeof(char**));
    for(int i=0;i<pow(2,s);i++){
        array[i]=(char**)calloc(E,sizeof(char*));
        for(int j=0;j<E;j++){
            array[i][j]=(char*)calloc(m,sizeof(char));
            for(int k=0;k<m;k++){
                array[i][j][k]=0;
            }
        }
    }
    queue_t **cq = cacheq_new(pow(2,s),E);
    /* parse trace file */
    while(fscanf(pFile," %c %lx, %d",&operation, &address, &size)>0){
        if (operation!='I')
            hit(array, address, operation, size, s, E, b, verbose, cq);
    }
    cacheq_free(cq,pow(2,s));
    /* free cache */
    for(int i=0;i<pow(2,s);i++)
    {
        for(int j=0;j<E;j++)
        {
            free(array[i][j]);
        }
    }
    for(int i=0;i<pow(2,s);i++)
    {
        free(array[i]);
    }
    free(array);
}
queue_t **cacheq_new(int i, int j){
    queue_t **q = malloc(sizeof(queue_t*)*i);
    for (int k = 0; k < i; ++k) {
        q[k] = malloc(sizeof(queue_t));
        if (q[k]==NULL)
            return NULL;
        q[k]->head = NULL;
        q[k]->tail = NULL;
    }
    for (int k = 0; k < i; ++k) {
        for (int l = 0; l < j; ++l) {
            char sp[10];
            sprintf(sp,"%d",l);
            q_insert_tail(q[k], sp);
        }
    }
    return q;
}
void cacheq_free(queue_t **q, int i){
    for (int k = 0; k < i; ++k) {
        if (q[k]==NULL) continue;
        list_ele_t *p = q[k]->head, *next = NULL;
        for (; p != NULL ; ) {
            next = p->next;
            free(p->value);
            free(p);
            p = next;
        }
        free(q[k]);
    }
}
void hit(char ***array, unsigned long int add, char operation, int size, int s, int E, int b, int verbose, queue_t **cq){
    /* determine tag bits */
    unsigned long int add_tag = add >> (s+b);
    /* determine set index bits */
    int t = 64 - s - b; // 64 bits address, tag bits equals to 64-s-b;
    unsigned long int set = (add << t) >> (64 - s);
    /* determine miss or not */
    bool hit = false, cold = false; // empty cache at first, I call it cold
    /* (valid bit, tag paired)   one(1, 1)      (1, 0) && (0, 0/1)                all(1, 0)
     *            operations L         hit                    miss            miss eviction
     *                       S         hit                    miss            miss eviction
     *                       M   one(1, 1) -> one(1, 1)            *****            hit hit
     *                           (1, 0) && (0, 0/1) -> one(1, 1)   *****           miss hit
     *                           all(1, 0) -> one(1, 1)            *****  miss eviction hit
     *  valgrind monitor memory in format "[space]operation address,size"
     *  operation includes L, S and M as for data management. Referring to caching, L, S and M can cause hit, miss or
     *  eviction. The laws are summarized in the table:
     *  1. we first determine set bit, the first dimension of the array.
     *  2. then judge the valid bit in cache, say "cold cache" (I wish), and valid bit will be 0;
     *     turn to the next line(E, the second dimension of the array) and do so;
     *     if valid bits equals to 0, check tag bits. If paired, so called one(1, 1) in the table, return 2.2 otherwise;
     *
     *  maybe the situation one(1, 1) doesn't exist. When cache are still cold, there may be situation (0, 0/1) which
     *  means valid bit equals to 0, corresponding to miss. When all E lines results are (1, 0), we evict some old data,
     *  corresponding to miss eviction.
     *
     *  S operation are the same.
     *
     *  M operation equals to L -> S, after ->, there would be hit
     */

    /*    one(1, 1)      (1, 0) && (0, 0/1)                   all(1, 0)
     * hit, no cold            cold, no hit             no hit, no cold
     * */

    int i = 0;
    for(;i<E;i++){
        /* check valid bit equals to 0, if so, cache is still cold and turn to the next line */
        if (!array[set][i][0]){
            cold = true;
            continue;
        }
        /* cache_tag start from array[set][i][1], 8 bytes. */
        unsigned long int * cache_tag = (unsigned long *) &array[set][i][1];
        if(*cache_tag==add_tag){
            hit = true;
            break;
        }
    }
    if(hit) {
        if (operation == 'M') {
            nhit++;
            if(verbose) printf("%c %lx,%d hit hit\n", operation, add, size);
        } else if (verbose) printf("%c %lx,%d hit\n", operation, add, size);

        nhit++;
        char sp[10];
        sprintf(sp, "%d", i);
        q_restart(cq[set], sp);
    }
    else if (cold){
        if (operation == 'M') {
            nhit++;
            if(verbose) printf("%c %lx,%d miss hit\n", operation, add, size);
        } else if (verbose) printf("%c %lx,%d miss\n", operation, add, size);
        nmiss++;
        for (int i = 0; i < E; ++i) {
            if (!array[set][i][0]){
                array[set][i][0]=1;
                unsigned long int * cache_tag = (unsigned long *) &array[set][i][1];
                *cache_tag = add_tag;
                char sp[10];
                sprintf(sp,"%d",i);
                q_restart(cq[set], sp);
                break;
            }
        }

    } else{
        if (operation == 'M') {
            nhit++;
            if(verbose) printf("%c %lx,%d miss eviction hit\n", operation, add, size);
        } else if (verbose) printf("%c %lx,%d miss eviction\n", operation, add, size);
        nmiss++;
        nevict++;
        char sp[10];
        q_remove_head(cq[set],sp,2);
        q_insert_tail(cq[set],sp);

        int i;
        sscanf(sp,"%d",&i);
        unsigned long int * cache_tag = (unsigned long *) &array[set][i][1];
        *cache_tag = add_tag;

    }

}
