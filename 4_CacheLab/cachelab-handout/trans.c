/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    /* for 32 bytes per block, set 8 * 8 ints block,
     * for A matrix, it occupies 8 sets (one fourth of the cache)
     * for B matrix, so as the situation in A */
    int bsizen = 8, bsizem = 8;
    int enn = bsizen * ( N / bsizen );
    int enm = bsizem * ( M / bsizem );
    int i, j, ii, jj, tmp;
    /* three for
    * first: major part of A and B, if else statement distinguishes
    * which blocks diagonally cause conflict miss, and it's the deformation
    * of function trans_db as involving more small blocks where the location
    * of the first elements no longer always be 0, as (ii,jj) shift exists.
    * the following code set bsize=8 as default, and can be still modified
    * second: lower left corner
    * third: upper right corner
    * fourth: lower right corner
    * */
    if(M==32&&N==32){
        for (ii = 0; ii < enn; ii += bsizen){
            for (jj = 0;  jj < enm; jj += bsizem) {
                if(ii!=jj) {
                    for (i = ii; i < ii + bsizen; i++) {
                        for (j = jj; j < jj + bsizem/2; j++) {
                            tmp = A[i][j];
                            B[j][i] = tmp;
                        }
                    }
                    for (i = ii+bsizen-1; i >= ii; i--) {
                        for (j = jj+bsizem/2; j < jj + bsizem; j++) {
                            tmp = A[i][j];
                            B[j][i] = tmp;
                        }
                    }
                }else {
                    for (i = ii; i < ii + bsizen; i++) {
                        for (j = jj; j < jj+bsizem; j++) {
                            tmp = A[i][j];
                            B[2*ii+7-i][2*jj+7-j] = tmp;
                        }
                    }
                    for (i = ii; i < ii + bsizen/2; i++) {
                        for (j = jj; j < jj+ii+bsizem; j++) {
                            tmp = B[i][j];
                            B[i][j] = B[2*jj+7-j][2*ii+7-i];
                            B[2*jj+7-j][2*ii+7-i] = tmp;
                        }
                    }
                }
            }
        }
    }
    else if(M==64&&N==64){
        for (ii = 0; ii < enn; ii += bsizen){
            for (jj = 0;  jj < enm; jj += bsizem) {
                if(ii!=jj) {
                    for (i = ii; i < ii + bsizen; i++) {
                        for (j = jj; j < jj+bsizem; j++) {
                            B[ii+jj+7-i][ii+jj+7-j] = A[i][j];
                        }
                    }
                    for (i = jj; i < jj + bsizen; i++) {
                        for (j = ii; j < jj+ii+bsizem-i; j++) {
                            tmp = B[i][j];
                            B[i][j] = B[ii+jj+7-j][ii+jj+7-i];
                            B[ii+jj+7-j][ii+jj+7-i] = tmp;
                        }
                    }
                }else {
                    for (i = ii; i < ii + bsizen; i++) {
                        for (j = jj; j < jj+bsizem; j++) {
                            tmp = A[i][j];
                            B[2*ii+7-i][2*jj+7-j] = tmp;
                        }
                    }
                    for (i = ii; i < ii + bsizen; i++) {
                        for (j = jj; j < jj+ii+bsizem-i; j++) {
                            tmp = B[i][j];
                            B[i][j] = B[2*jj+7-j][2*ii+7-i];
                            B[2*jj+7-j][2*ii+7-i] = tmp;
                        }
                    }
                }
            }
        }
    }
    for (jj = 0;  jj < enm; jj += bsizem) {
        for (i = enn; i < N; i++) {
            for (j = jj; j <jj+bsizem; j++) {
                tmp = A[i][j];
                B[j][i] = tmp;

            }
        }
    }
    for (ii = 0; ii < enn; ii += bsizen){
        for (j = enm;  j< M; j++) {
            for (i = ii;  i< ii+bsizen; i++) {
                tmp = A[i][j];
                B[j][i] = tmp;
            }
        }
    }
    for (i = enn; i < N; i++) {
        for (j = enm;  j< M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}
/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}
char trans_db_desc[] = "Small trace block transpose";
void trans_db(int M, int N, int A[N][M], int B[M][N])
{
    /* this function is designed for small trace block transpose,
     * when bsize = 8, block transpose for N, M = 8, 16, 24, 32 miss for
     * 40   56  72  88
     * 56   109 136 172
     * 72   136 270 256
     * 88   172 256 343
     * Ideal miss: 256 when N = M = 32, because (A size + B size)/ blocksize
     * = 32*32*2/(32bytes/4bytes_per_int) = 256
     * so whats the problem?
     * each block transpose miss = 256/16 = 16
     * and the results in the first line and first column follow the ideal law
     * but how about trace block, 40 109, much more than 16!
     * conflict miss!
     * in debugging tracegen, A addr 0x55d6d8b3f080
     *                        B      0x55d6d8b7f080
     *                        0x40000 bytes difference
     *      in tracegen.c static int A[256][256], this make sense.
     *      for convenience, take the last five bytes
     *      0x3f080           0011 1111 0000 1000 0000
     *      0x7f080           0111 1111 0000 1000 0000
     *      s = b = 5 last ten bits are the same, conflict miss
     * this function is designed for small trace block transpose!!!
     */
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            B[7-i][7-j] = A[i][j];
        }
    }
    for (i = 0; i < N; i++) {
        for (j = 0; j < 8-i; j++) {
            tmp = B[i][j];
            B[i][j] = B[7-j][7-i];
            B[7-j][7-i] = tmp;
        }
    }
}
/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

