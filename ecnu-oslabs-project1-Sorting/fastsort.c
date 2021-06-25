//10142130214
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>

int cmp(const void* a, const void* b) {
	return ((rec_t *)a)->key > ((rec_t *)b)->key ? 1 : -1;
}

int main(int argc, char *argv[]) {
	//the number of arguments must be 3
	if (argc != 3) {
		fprintf(stderr, "Usage: fastsort inputfile outputfile\n");
    	exit(1);
	}
	
	//open the two files
	char *inFile = argv[1];
	char *outFile   = argv[2];
	int infd = open(inFile, O_RDONLY);
	int outfd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
	
	//print error infomation if can not open the files
	if (infd < 0) {
		fprintf(stderr, "Error: Cannot open file %s\n", inFile);
		exit(1);
    }
    if (outfd < 0) {
		fprintf(stderr, "Error: Cannot open file %s\n", outFile);
		exit(1);
    }
    
    //get the size of inputfile and allocate space to the array
    struct stat buf;
    fstat(infd, &buf);
	rec_t* r = (rec_t*)malloc(sizeof(rec_t) * buf.st_size / 100);
	
	//read the data and store them in the array
	int cnt = 0;
    while (1) {
		int rc;
		rc = read(infd, &r[cnt], sizeof(rec_t));
		if (rc == 0)
			break;
		if (rc < 0) {
			fprintf(stderr, "Error: Cannot read\n");
			exit(1);
		}
		cnt++;
    }
    
    //sort the array based on the key and write into outputfile
    qsort(r, cnt, sizeof(rec_t), cmp);
    for (int i = 0; i < cnt; ++i) {
    	int rc = write(outfd, &r[i], sizeof(rec_t));
		if (rc != sizeof(rec_t)) {
			fprintf(stderr, "Error: Cannot write\n");
			exit(1);
		}
    }
    
    //do not forget to close the files
    close(infd);
    close(outfd);
	return 0;
}
