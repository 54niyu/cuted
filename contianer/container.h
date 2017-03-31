/*
 *	create by Bing
 *	implemetion of contianers 
 *	vector
 *	string
 *	hashmap
 */
#ifndef __CONTAINER_H_
#define __CONTAINER_H_

#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#define VECTOR_DEFAULT_CAP 8

typedef struct {
    void *array;
    unsigned int len;
    unsigned int cap;
    short int size;
} vector;

vector *vectorMakeSize(int len, int size);

vector *vectorMake(int size);

void vectorDestroy(vector *v);

void *vectorIn(vector *v, int idx);

void *vectorPush(vector *v, void *val);

void vectorPop(vector *v);

// ---------------------------------------------
typedef struct {
    char *str;
    unsigned int len;
} string;

// ---------------------------------------------

typedef struct _bucket {
    struct _bucket *next;
    void *key;
    void *val;
} bucket;

typedef struct {
    unsigned int count;
    unsigned int oldcount;
    unsigned int size;
    bucket **buckets;
    bucket **oldbuckets;

    int (*hash)(void *);

    int (*compare)(void *, void *);

    void *(*copyKey)(void *);

    void (*deleteKey)(void *);
} map;

map *mapMake();

bucket *bucketMake();

void *mapFind(map *m, void *key);

void *mapInsert(map *m, void *key, void *val);

void mapPrint(map* m);

#endif
