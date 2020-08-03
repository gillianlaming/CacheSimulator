#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include "cachelab.h"
#include <math.h>
#include <limits.h>
/*This file simulates a cache*/

#define BIGNUMBER 1000
enum {
  SUCCESS,
  COMMAND_LINE_FAIL,
  FILE_FAIL
};
typedef struct {
  int hitCount;
  int missCount;
  int evictions;
} stats;

typedef struct {
  int h;
  int v;
  int s;
  int S; /*number of sets in the cache*/
  int E; /*lines per set in the cache*/
  int b;
  int B; /*block size*/
  int t;
} argFlags;

typedef struct{
  int valid;
  int identifier;
  unsigned long long int tag;
}cacheLine;

typedef struct{
  cacheLine *cacheLines;
}cacheSet;

argFlags flags = {
  .h = 0,
  .v = 0,
  .s = 0,
  .E = 0,
  .b = 0,
  .t = 0,
};

int temp;
int id = 0;
signed int size;

char operation;
unsigned long long int address;
char *filePath;
char readLine[BIGNUMBER];

void openFile();
void printUsage();
void setupCache();
void clearCache();
void checkCache(unsigned long long int address);

stats myStats;
cacheSet *myCache;
cacheSet mySet;
cacheLine myLine = {
  .valid = 0,
  .tag = -1,
  .identifier = 0,
};

int main(int argc, char* const argv[])
{
   myStats.hitCount = myStats.missCount = myStats.evictions = 0;
   char argVal;
   char  options[] = "hvs:E:b:t:";
   
   while((argVal=getopt(argc, argv, options)) != -1)
   {
     switch(argVal){
       case 'h': 
	 flags.h = 1;
	 exit(SUCCESS);
	 break;
       case 'v':
	 flags.v = 1;
	 break;
       case 's':
	 flags.s  = atoi(optarg);
	 flags.S = 1 << flags.s; /*raise to power of 2*/
       	 break;
       case 'E':
     	 flags.E = atoi(optarg);
	 break;
       case 'b':
	 flags.b = atoi(optarg);
	 flags.B = 1 <<flags.b;/*raise to power 2*/
       	 break;
       case 't':
	 flags.t = 1;
	 filePath = optarg;
       	 break;
    }
   }
   /*help flagged. print and exit.*/
   if (flags.h == 1){
     printUsage(argv);
     exit(SUCCESS);
   }
   /* wrong number of command line args*/
   if (flags.s == 0
       || flags.E==0
       || flags.b==0
       || flags.t == 0){
     printUsage(argv);
     exit(COMMAND_LINE_FAIL);
   }
   setupCache();
   openFile();
   printSummary(myStats.hitCount, myStats.missCount, myStats.evictions);
   clearCache();
   return SUCCESS;
}

void openFile(){
  FILE *in;
  in = fopen(filePath, "r");
  if(!in){
     exit(FILE_FAIL);
  }
  while(fgets(readLine, BIGNUMBER, in) != NULL){
      sscanf(readLine," %c %llx,%d", &operation,&address,&size);
      
      switch(operation){
        case('M'):
       	  checkCache(address);
	  checkCache(address);
      	  break;
        case('S'):
	  checkCache(address);
	  break;
        case('L'):
	  checkCache(address);
	  break;
        default:
	  break;
      }
   }
   fclose(in);
}

void printUsage(char* argv[]){
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n");
  printf("\nExamples:\n");
  printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
  printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
}

/*allocate storage and set up cache*/
void setupCache(){
  myCache = malloc(flags.S * sizeof(cacheSet));

  /*for each set in the cache*/
  for(unsigned int i = 0; i < flags.S; i++){
    mySet.cacheLines = malloc(sizeof(cacheLine)*flags.E);
    myCache[i] = mySet;

    /*for each line in the set*/
    for (unsigned int j = 0; j < flags.E; j++){
      mySet.cacheLines[j] = myLine;
    }
  }
}

void checkCache(unsigned long long int address){
  unsigned long long int tag = address >> (flags.s + flags.b); 
  int min = INT_MAX; 
  int size = (64 - (flags.s + flags.b));
  unsigned long long int shifted = address << (size);
  unsigned long long int setIndex = shifted >> (size + flags.b);
  int emptyLine = -1;
  int evictLine = -1;
  
  /*need to compare to the address of the lines in our cache*/
  cacheSet targetSet = myCache[setIndex];
  for (unsigned int i = 0; i < flags.E; i++){
    
    cacheLine theLine = targetSet.cacheLines[i];
    if (theLine.valid == 1){
        /*hit!*/
        if (theLine.tag == tag){
       	  myStats.hitCount++;
	  theLine.identifier++;
	  myCache[setIndex].cacheLines[i] = theLine;
	  targetSet.cacheLines[i] = theLine;
	  targetSet.cacheLines[i].identifier = id++;
	  return;
	}
	/*if no hit, find least recently used to evict*/
	else if (theLine.identifier < min){
	  evictLine = i;
	  min = theLine.identifier;
      	}	
    }
    else {
      /*set the empty space to that index*/
      if(emptyLine == -1 ){
      	emptyLine = i;
      }
    }
  }
    myStats.missCount++;

    /*there exists an empty line*/
    if (emptyLine != -1){
      targetSet.cacheLines[emptyLine].tag = tag;
      targetSet.cacheLines[emptyLine].valid = 1;
      targetSet.cacheLines[emptyLine].identifier = id++;
    }
    /*otherwise need to evict a line*/
    else {
      myStats.evictions++;
      targetSet.cacheLines[evictLine].tag = tag;
      targetSet.cacheLines[evictLine].identifier = id++;
    }
}
void clearCache(){
  free(mySet.cacheLines);
  free(myCache);
 }



