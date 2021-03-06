#include "container.h"

void test_vector() {
    vector *v = vector_make_size(1, sizeof(int));
    int a[] = {1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15,16};
    *(int *) vector_in(v, 0) = 12;
    int i = 0;
    for (; i < 15; i++) {
        vector_push(v, a + i);
        printf("-- %d %d\n",a[i], v->len);
    }
    for (i = 0; i < 15; i++) {
        vector_push(v, &i);
    }
    printf("%d\n",v->len);
    for (i = 0; i < (int)v->len; i++) {
        printf("%d\n", *((int *) vector_in(v, i)));
    }
    for (i = 0; i < 50; i++) {
        vector_pop(v);
    }
    for (i = 0; i < (int)v->len; i++) {
        printf("%d\n", *((int *) vector_in(v, i)));
    }
    char c[] = "123456789";
    string b;
    b.str = c;
    b.len = 7;
    vector *v2 = vector_make(sizeof(string));
    memcpy(v2->array, &b, v2->size);
    printf("%s", ((string *) vector_in(v2, 0))->str);
}

int sdbmHash(void *key) {
    int hash = 0;
    char *str = (char *) key;
    while (*str) {
        // equivalent to: hash = 65599*hash + (*str++);
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }
    return (hash & 0x7FFFFFFF);
}

void *copyKey(void *key) {
    int len = strlen((char*)key);
    char *dst = (char *) malloc(len+1);
    if( dst ==  NULL){
        perror("malooc str");
        return NULL;
    }else{
        strcpy(dst, (char *)key);
    }
    return dst;
}


int compareKey(void *a, void *b) {
    return strcmp((char *) a, (char *) b);
}

void test_map() {

    map *m = map_make();
    m->hash = sdbmHash;
    m->copyKey = copyKey;
    m->compare = compareKey;

    char *test[] = {
            "asd",
            "bsddj",
            "csdf",
            "dsdf",
            "esdafasd",
            "fasdf",
            "gsadf",
            "gs",
            "fuck",
            "safsd",
            "sadfasd",
            "asddfasdf",
            "asdfasdf",
            "sadfe",
            "asdfasd",
            "erer",
            "rtert",
            "asdf",
            "4erwerw",
            "sawe23",

    };
    int a[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};

    int i = 0;
    for (; i < 200; i++) {
        map_set(m, test[i % 17], a + i % 17);
    }

    printf("Over\n");
    for (i = 0; i < 200; i++) {
        bucket* val = map_get(m, test[i % 17]);
        if (val == NULL) {
            printf("not found");
        } else {
//            printf(" %p %s %d\n", val,(char*)val->key, *((int*)(val->val)));
        }
    }
    map_print(m);
}

int main() {

    test_map();
    test_vector();
    return 0;

}


