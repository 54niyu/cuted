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

vector *vector_make_size(int len, int size);

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

map *map_make();

bucket *bucket_make();

void *map_find(map *m, void *key);

void *map_insert(map *m, void *key, void *val);

void map_print(map *m);

#endif
