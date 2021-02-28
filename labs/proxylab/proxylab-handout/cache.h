// cache.h
#include <sys/time.h>
#include "csapp.h"

#define TYPES 6
extern const int cache_block_size[];
extern const int cache_cnt[];

typedef struct cache_block{
    char* url;
    char* data;
    int datasize;
    int64_t time;
    pthread_rwlock_t rwlock;
} cache_block;

typedef struct cache_type{
    cache_block *cacheobjs;  
    int size;
} cache_type;

cache_type caches[TYPES];

//intialize cache with malloc
void init_cache();
//if miss cache return 0, hit cache write content to fd
int read_cache(char* url, int fd);
//save value to cache
void write_cache(char* url, char* data, int len);
//free cache
void free_cache();