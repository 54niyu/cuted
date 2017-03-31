#include "container.h"

bucket *bucketMake() {
    bucket *b = (bucket *) malloc(sizeof(bucket));
    if (b == NULL) {
        perror("malloc bucket");
    }
    return b;
}

void *mapFind(map *m, void *key) {

    bucket **idx = NULL;
    bucket *b = NULL;
    bucket *pre = NULL;
    // find in oldbucket
    if (m->oldbuckets != NULL) {
        int hash = m->hash(key) % ((1 << (m->size - 1)) - 1);
        idx = m->oldbuckets + hash;
        b = *(idx);
        pre = b;
        while (b != NULL) {
            if (m->compare(key, b->key) != 0) {
                pre = b;
                b = b->next;
            } else {
                break;
            }
        }
        if (b != NULL) {
            // move b to new buckets
            // new key idx
            printf("Move\n");
            hash = m->hash(key) % ((1 << (m->size)) - 1);
            bucket **pos = m->buckets + hash;
            // remove from old bucket
            if (b == pre) {
                *idx = b->next;
            } else {
                pre->next = b->next;
            }
            // pre->next = b->next;
            // add to new bucket
            b->next = *pos;
            *pos = b;

            (m->oldcount)--;
        }
    }
    if (m->oldbuckets != NULL && m->oldcount == 0) {
        free(m->oldbuckets);
        m->oldbuckets = NULL;
        printf("remove old buckets\n");
    }
    if (b != NULL) {
        return b;
    } else {
        // find in new buckets
        int hash = m->hash(key) % ((1 << m->size) - 1);
        idx = m->buckets + hash;
        b = *idx;
        while (b != NULL) {
            if (m->compare(key, b->key) != 0) {
                b = b->next;
            } else {
                return b;
            }
        }
        return NULL;
    }
}

void *mapGrow(map *m) {
    // twice bucket size
    printf("grow %d %d %d\n", m->size, m->count, m->oldcount);
    if (m->oldbuckets != NULL && m->oldcount != 0) {
        printf("Old is not empty\n");
        return NULL;
    }
    m->size++;
    m->oldbuckets = m->buckets;
    m->oldcount = m->count;
    m->buckets = (bucket **) calloc((1 << (m->size)), sizeof(bucket *));
    return NULL;
}

void *mapInsert(map *m, void *key, void *val) {
    if ((m->count * 1.0 / (1 << m->size)) > 0.65) {
        // rehash
        mapGrow(m);
    }
    bucket *b = mapFind(m, key);
    if (b != NULL) {
        // overrite value
        b->val = val;
    } else {
        // add new key/val
        int hash = m->hash(key) % ((1 << m->size) - 1);
        bucket **pos = NULL;
        pos = m->buckets + hash;

        b = bucketMake();
        b->key = m->copyKey(key);
        b->val = val;
        b->next = *pos;
        *pos = b;
        m->count++;
    }
    return NULL;
}

map *mapMake() {
    map *m = (map *) calloc(1, sizeof(map));
    m->size = 3;
    m->count = 0;
    m->buckets = (bucket **) calloc(8, sizeof(bucket *));
    return m;
}


void bucketPrint(bucket **b, int sz) {
    int i = 0;
    for (; i < sz; i++) {
        bucket **p = b + i;
        if (*p != NULL) {
            printf("bucket %d ", i);
            bucket *pr = *p;
            while (pr != NULL) {
                printf("%s %d ->", (char *) pr->key, *((int *) pr->val));
                pr = pr->next;
            }
            printf("\n");
        }
    }
}

void mapPrint(map *m) {
    printf("%d %d %d\n", m->size, m->count, m->oldcount);
    if (m->oldbuckets != NULL) {
        bucketPrint(m->oldbuckets, (1 << (m->size - 1)) - 1);
    }
    printf("New buckets\n");
    bucketPrint(m->buckets, (1 << (m->size)) - 1);
}
