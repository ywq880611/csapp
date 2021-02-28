#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#define max_count 10000

int misses, evictions, hits;

typedef struct sLine
{
    bool valid;
    int tag;
    int count;
}Line;


typedef struct sSet
{
    Line* lines;
}Set;

typedef struct sCache{
    int set_num;
    int line_num;
    Set* sets;
}Cache;

void printHelpMenu(){
    printf("Help me.\n");
}

void checkOptarg(char* optarg){
    if(optarg[0] == '-'){
        printf("./csim: Missing required command line argument.\n");
        printHelpMenu();
        exit(0);
    }
}

int get_Opt(int argc, char** argv, int* s, int* E, int* b, char* fileName, bool* isVerbose){
    int c;
    while((c = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(c){
            case 'v':
                *isVerbose = true;
                break;
            case 's':
                checkOptarg(optarg);
                *s = atoi(optarg);
                break;
            case 'E':
                checkOptarg(optarg);
                *E = atoi(optarg);
                break;
            case 'b':
                checkOptarg(optarg);
                *b = atoi(optarg);
                break;
            case 't':
                checkOptarg(optarg);
                strcpy(fileName, optarg);
                break;
            case 'h':
                break;
            default:
                printf("invalid parameter %d.\n", c);
                printHelpMenu();
                exit(0);   
        }
    }
    return 1;
}

void updateCount(Cache* ca, int setbits, int linebits){
    ca->sets[setbits].lines[linebits].count = max_count;
    for(int i = 0; i < ca->line_num; i ++)
        if(i != linebits) ca->sets[setbits].lines[i].count --;
}

int findEvict(Cache* ca, int setbits){
    int minIndex = 0, minCount = max_count;
    for(int i = 0; i < ca->line_num; i ++)
        if(ca->sets[setbits].lines[i].count < minCount)
            minIndex = i, minCount = ca->sets[setbits].lines[i].count;
    return minIndex;
}

bool isMiss(Cache* ca, int setbits, int tagbits){
    for(int i = 0; i < ca->line_num; i ++){
        if(ca->sets[setbits].lines[i].valid == true && ca->sets[setbits].lines[i].tag == tagbits){
            updateCount(ca, setbits, i);
            return false;
        }
    }
    return true;
}

bool updateCache(Cache* ca, int setbits, int tagbits){
    bool isFull = true;
    int i;
    for(i = 0; i < ca->line_num; i ++){
        if(ca->sets[setbits].lines[i].valid == false){
            isFull = false;
            break;
        }
    }
    if(isFull){
        int evict = findEvict(ca, setbits);
        ca->sets[setbits].lines[evict].tag = tagbits;
        updateCount(ca, setbits, evict);
    }
    else{
        ca->sets[setbits].lines[i].valid = true;
        ca->sets[setbits].lines[i].tag = tagbits;
        updateCount(ca, setbits, i);
    }
    return isFull;
}

void loadData(Cache* ca, int setbits, int tagbits, bool isVerbose){
    if(isMiss(ca, setbits, tagbits)){
        misses ++;
        if(isVerbose) printf("miss ");
        if(updateCache(ca, setbits, tagbits)){
            evictions ++;
            if(isVerbose) printf("eviction ");
        }
    }
    else{
        hits ++;
        if(isVerbose) printf("hit ");
    }
}

void storeData(Cache* ca, int setbits, int tagbits, bool isVerbose){
    loadData(ca, setbits, tagbits, isVerbose);
}

void modifyData(Cache* ca, int setbits, int tagbits, bool isVerbose){
    loadData(ca, setbits, tagbits, isVerbose);
    storeData(ca, setbits, tagbits, isVerbose);
}

void init_Cache(int s, int E, int b, Cache* ca){
    if(s < 0){
        printf("invalid cache set number.");
        exit(0);
    }
    ca->set_num = 2 << s;
    ca->line_num = E;
    ca->sets = (Set*) malloc(ca->set_num * sizeof(Set));
    if(!ca->sets){
        printf("Set memory error.\n");
        exit(0);
    }
    int i, j;
    for(i = 0; i < ca->set_num; i ++){
        ca->sets[i].lines = (Line*) malloc(ca->line_num * sizeof(Line));
        if(!ca->sets[i].lines){
            printf("Line memory error.\n");
            exit(0);
        }
        for(j = 0; j < E; j ++){
            ca->sets[i].lines[j].valid = false;
            ca->sets[i].lines[j].count = 0;
        }
    }
}

int getSet(int addr, int s, int b){
    addr = addr >> b;
    int mask = (1 << s) - 1;
    return addr & mask;
}

int getTag(int addr, int s, int b){
    int mask = s + b;
    return addr >> mask;
}

int main(int argc, char** argv)
{
    int s, E, b;
    bool isVerbose = false;
    char fileName[100], opt[10];

    int addr, size;
    misses = evictions = hits = 0;
    Cache ca;
    
    get_Opt(argc, argv, &s, &E, &b, fileName, &isVerbose);
    init_Cache(s, E, b, &ca);
    FILE* tracefile = fopen(fileName, "r");
    
    while(fscanf(tracefile, "%s %x,%d", opt, &addr, &size) != EOF){
        if(strcmp(opt, "I") == 0) continue;
        int setbits = getSet(addr, s, b);
        int tagbits = getTag(addr, s, b);
        printf("------------------------\n");
        printf("setbits:%x tagbits:%x\n", setbits, tagbits);
        if(isVerbose) printf("%s %x,%d ", opt, addr, size);
        if(strcmp(opt, "L") == 0) loadData(&ca, setbits, tagbits, isVerbose);
        if(strcmp(opt, "S") == 0) storeData(&ca, setbits, tagbits, isVerbose);
        if(strcmp(opt, "M") == 0) modifyData(&ca, setbits, tagbits, isVerbose);
        if(isVerbose) printf("\n");
    }

    printSummary(hits, misses, evictions);
    return 0;
}
