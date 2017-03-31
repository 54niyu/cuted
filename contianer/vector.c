#include "container.h"

vector* vectorMakeSize(int len,int size){
    vector* v = (vector*)malloc(sizeof(vector));
    if( v == NULL){
	perror("vector malloc");
        return NULL;
    }
    (*v).array = (void *)malloc(len * size);
    if((*v).array == NULL){
	perror("vector array malloc");
       free(v);
       return NULL;
    }
    (*v).size = size;
    (*v).len = len;
    (*v).cap = len;
    return v;
}

vector* vectorMake(int size){
    return vectorMakeSize(VECTOR_DEFAULT_CAP, size);
}

void vectorDestroy(vector* v){
    if(v != NULL){
        free((*v).array);
    }
    free(v);
}


void* vectorIn(vector* v,int idx){
    if(idx < 0 || (unsigned int)idx > (*v).len ){
        return NULL;
    }
    return (*v).array +  idx * ((*v).size);
}

void* vectorPush(vector* v,void* val){
    if ( (*v).len + 1 > (*v).cap){
        void* narry = realloc((*v).array, (*v).cap * (*v).size * 2);
        if( narry == NULL){
	    perror("vector realloc");
	    vectorDestroy(v);
        }else{
            (*v).array = narry;
            (*v).cap *= 2;
        }
    }
    memcpy((*v).array + (*v).len * (*v).size, val, (*v).size);
    (*v).len += 1;
    return NULL;
};

void vectorPop(vector* v){
    if(v == NULL)   return ;
    if( (*v).len <= 0){
        return ;
    }
    (*v).len -= 1;
}

