#include "debug.h"
#include "arraylist.h"
#include "foreach.h"
#include <pthread.h>
#include <string.h>

//We need a global variable key for this
static pthread_key_t key;
static pthread_once_t once = PTHREAD_ONCE_INIT;

void init_routine(){
    pthread_key_create(&key, NULL);
}

void *foreach_init(arraylist_t *self){
    if(self->length == 0){
        return NULL;
    }
    void *ret = NULL;
    int* thread_index = malloc(sizeof(int));
    *thread_index = 0;
    pthread_once(&once, init_routine);
    pthread_setspecific(key, thread_index);
    ret = get_index_al(self, 0);
    sem_wait(&self->semaphore);
    self->foreach_loopers++;
    sem_post(&self->semaphore);
    return ret;
}

void *foreach_next(arraylist_t *self, void* data){
    void *ret = NULL;
    int* current_index = pthread_getspecific(key);
    if(data != NULL){
        void* index_address = (char*)self->base + (*current_index * self->item_size);
        sem_wait(&self->semaphore);
        memcpy(index_address, data, self->item_size);
        sem_post(&self->semaphore);
    }

    if(*current_index+1 == self->length){
        sem_wait(&self->semaphore);
        self->foreach_loopers--;
        sem_post(&self->semaphore);
        return NULL;
    } else {
        (*current_index)++;
        pthread_setspecific(key, current_index);
        ret = get_index_al(self, *current_index);
    }
    return ret;
}

size_t foreach_index(){
    size_t ret = 0;
    int* index = pthread_getspecific(key);
    ret = *index;
    return ret;
}

bool foreach_break_f(){
    bool ret = true;
    int* index = pthread_getspecific(key);
    free(index);
    return ret;
}

int apply(arraylist_t *self, int(*application)(void*)){
    int ret = 0;
    foreach(void, list_item, self){
        int perform_application = application(list_item);
        if(perform_application == -1){
            list_item = NULL;
        }
    }
    return ret;
}
