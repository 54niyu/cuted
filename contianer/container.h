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
    unsigned int size;
} vector;

vector *vector_make_size(unsigned int len, unsigned int size);

vector *vector_make(int size);

void vector_destroy(vector *v);

void *vector_in(vector *v, int idx);

void *vector_push(vector *v, void *val);

void vector_pop(vector *v);

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

map* map_make();
bucket* bucket_make();

void* map_get(map *m, void *key);
void* map_set(map *m,void *key, void *nval);
void* map_ite(map *m);
void  map_del(map *m,void *key);
void  map_print(map *m);

#endif
