#include "container.h"

vector* vector_make_size(unsigned int len, unsigned int size){
    vector* v = (vector*)malloc(sizeof(vector));
    if( v == NULL){
	perror("vector malloc");
        return NULL;
    }
    v->array = (void *)malloc(len * size);
    if(v->array == NULL){
       perror("vector array malloc");
       free(v);
       return NULL;
    }
    v->size = size;
    v->len = len;
    v->cap = len;
    return v;
}

vector* vector_make(int size){
    return vector_make_size(VECTOR_DEFAULT_CAP, size);
}

void vector_destroy(vector *v){
    if(v != NULL){
        free(v->array);
    }
    free(v);
}


void* vector_in(vector *v, int idx){
    if(idx < 0 || (unsigned int)idx > v->len ){
        return NULL;
    }
    return v->array +  idx * (v->size);
}

void* vector_push(vector *v, void *val){
    if ( v->len + 1 > v->cap){
        void* narry = realloc(v->array, v->cap * v->size * 2);
        if( narry == NULL){
	    perror("vector realloc");
            vector_destroy(v);
        }else{
            v->array = narry;
            v->cap *= 2;
        }
    }
    memcpy(v->array + v->len * v->size, val, v->size);
    v->len += 1;
    return NULL;
};

void vector_pop(vector *v){
    if(v == NULL)   return ;
    if( v->len <= 0){
        return ;
    }
    v->len -= 1;
}

