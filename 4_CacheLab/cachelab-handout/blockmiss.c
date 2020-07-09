#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
/* the address of first element in A matrix*/
unsigned int afe = 0x10d0a0;
unsigned int bfe = 0x14d0a0;
/* block matrix, nmiss of each block*/
unsigned int bnm[8][8];
void usage(char *argv[]){
    printf("This program is designed for CMU15213 cachelab out of interest.\
For matrix transpose, dividing the matrix into blocks is a \
candidate approach. Then transpose each element in the blocks.\
Hence, some problem exists such as thrash.\
Luckily, I can moniter every block reading or writing with command\
valgrind.\
This program parses each line of trace files and recording nmiss\
or nhits when reading from A matrix\n");
    printf("Usage: %s [-h] -f [-p] [-r]\n",argv[0]);
    printf("Options:\n");
    printf("  -h        print this Help message.\n");
    printf("  -f        input trace File.\n");
    printf("  -p        oPerations. reading(0) or writing (1)? default: 0\n");
    printf("  -r        hit (1) Or miss (0) to record? default: 0\n");
}
void blockmiss(FILE* fp,int p, int r){
    unsigned int ad,size,nmiss;
    char horm,op;
    int diff,quotient1,quotient2,remainder;
    while(fscanf(fp,"%c %x,%d %c",&op,&ad,&size,&horm)>0)
    {
        if (op=='L') diff = ad-afe;
        else if (op=='S') diff = ad-bfe;
        if((op=='L'&&p==0)||(op=='S'&&p==1)){
            diff = diff/4;
            quotient1 = diff/0x200;//determine the line of the block matrix
            remainder = diff%0x200;
            remainder = remainder%0x40; //for determing the column, move to the first of each block;
            quotient2 = remainder/8;
            if(quotient1>=0 && quotient1<8 && quotient2>=0 && quotient2<8){
                if((horm=='h'&&r==1)||(horm=='m'&&r==0)){
                    bnm[quotient1][quotient2]=bnm[quotient1][quotient2]+1;
                }
            }
        }
    }
    if(p) printf("writing\n");
    else printf("reading\n");
    if(r) printf("nhits\n");
    else printf("nmisses\n");
}
void printmatrix(unsigned int bnm[8][8]){
    for(int i=0; i<8; i++){
        for(int j=0; j<8;j++){
            printf("    %d",bnm[i][j]);
        }
    printf("\n");
    }
}


int main(int argc, char** argv){
	int opt,ch;
	char* file;
	FILE* fp;
    int op=0, r=0;
	while ((opt=getopt(argc,argv,"hf:p:r:"))!=-1){
		switch(opt){
			case 'h':
				usage(argv);
                exit(0);
            case 'f':
                file = optarg;
                break;
            case 'p':
                op = atoi(optarg);
                break;
            case 'r':
                r = atoi(optarg);
                break;
            default:
                usage(argv);
                exit(1);
        }
    }
    fp = fopen(file,"r");
    assert(fp);
    blockmiss(fp,op,r);
    fclose(fp);
    printmatrix(bnm);
}

