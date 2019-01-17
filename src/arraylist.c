#include "arraylist.h"
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

/**
 * @visibility HIDDEN FROM USER
 * @return     true on success, false on failure
 */
static bool resize_al(arraylist_t* self){
    bool ret = false;
    if(self->freed){
        return false;
    }
    //Grow by reallocation of the memory
    if(self->length == self->capacity){
        self->base = realloc(self->base, (self->capacity*2)*self->item_size);
        if(self->base == NULL){
            errno = ENOMEM;
            return false;
        }
        self->capacity = self->capacity * 2;
        ret = true;
    }
    //Shrink
    else if(self->length == (self->capacity/2 -1)) {
        if(self->capacity == INIT_SZ || (self->capacity)/2 < INIT_SZ){
            errno = EINVAL;
            return false;
        }
        self->base = realloc(self->base, (self->capacity*2)*self->item_size);
        if(self->base == NULL){
            errno = ENOMEM;
            return false;
        }
        self->capacity = self->capacity/2;
        ret = true;
    }
    return ret;
}

/*
*   Do not have to use a semaphore for creation of an array because
*   if we create a new array then all of the information about the array
*   is local. If the array is global then all methods modifying it will
*   require usage of th semaphore. Initialize the semaphore on creation.
*/
arraylist_t *new_al(size_t item_size){
    void *ret = NULL;
    //Check to see if there wasn't enough space to allocate for the array
    if((ret = calloc(INIT_SZ, item_size)) == NULL){
        errno = ENOMEM;
        return NULL;
    }
    //Allocate space for the struct of the arraylist_t
    arraylist_t* head = malloc(sizeof(arraylist_t));
    if(head == NULL){
        errno = ENOMEM;
        return NULL;
    }
    head->capacity = INIT_SZ;
    head->length = 0;
    head->item_size = item_size;
    head->base = ret;
    head->freed = false;
    head->num_readers = 0;
    head->foreach_loopers = 0;
    sem_init(&(head->semaphore), 0, 1);
    sem_init(&head->readers, 0, 1);
    return head;
}

/*
*   On insertion, local variables are ret and resize_success
*   which do not require usage of semaphore. Must wrap resize_al
*   within a post and wait. This is a writer function.
*/
size_t insert_al(arraylist_t *self, void* data){
    size_t ret = UINT_MAX;
    bool resize_success;

    //Check to see if we are at max capacity of the arraylist
    //If so then resize. On false, set errno to ENOMEM and return UINT_MAX
    /*if(self->length == self->capacity){
        sem_wait(&(self->semaphore));
        resize_success = resize_al(self);
        sem_post(&(self->semaphore));
        if(resize_success == false || self->freed){
            errno = ENOMEM;
            return ret;
        }
    }*/

    //Must wait and post because we are referencing shared data about an array
    sem_wait(&(self->semaphore));
    if(self->freed){
        errno = ENOMEM;
        sem_post(&self->semaphore);
        return UINT_MAX;
    }
    if(self->length == self->capacity){
        resize_success = resize_al(self);
        if(resize_success == false){
            errno = ENOMEM;
            sem_post(&self->semaphore);
            return UINT_MAX;
        }
    }
    void* array_index = self->base;
    array_index = ((char*)array_index) + self->item_size * self->length;
    memcpy(array_index, data, self->item_size);
    self->length++;
    ret = self->length - 1;
    sem_post(&(self->semaphore));

    return ret;
}

//THIS IS A READER FUNCTION
size_t get_data_al(arraylist_t *self, void *data){
    size_t ret = 0;
    bool found = false;

    sem_wait(&self->readers);
    self->num_readers++;
    if(self->num_readers == 1 && !self->freed){
        sem_wait(&self->semaphore);
    }
    sem_post(&self->readers);

    if(self->length == 0 || self->freed){
        errno = EINVAL;
        return UINT_MAX;
    }
    //From this point on, length is greater than or equal to 1
    //Return index 0 if data is NULL;
    if(data == NULL){
        return 0;
    } else {
        for(int i = 0; i < self->length; i++){
            void* array_data = (char*)self->base + (i * self->item_size);
            if(memcmp(array_data, data, self->item_size) == 0){
                found = true;
                break;
            }
            ret++;
        }
    }

    sem_wait(&self->readers);
    self->num_readers--;
    if(self->num_readers == 0){
        sem_post(&self->semaphore);
    }
    sem_post(&self->readers);

    //Return false if we could not find the data. Return the index otherwise
    /*
    *   We do not have to use a semaphore if we are evaluating local variables.
    */
    if(found == false){
        errno = EINVAL;
        return UINT_MAX;
    } else {
        return ret;
    }
}

//THIS IS A READER FUNCTION
void *get_index_al(arraylist_t *self, size_t index){
    void *ret = NULL;
    void* indexed_mem;
    ret = malloc(sizeof(self->item_size));

    //MUST PROTECT READERS COUNT AND PREVENT WRITING
    sem_wait(&self->readers);
    self->num_readers++;
    if(self->num_readers == 1 && !self->freed){
        sem_wait(&self->semaphore);
    }
    sem_post(&self->readers);

    if((int)index < 0 || self->length == 0 || self->freed){ //ERROR CASE
        errno = EINVAL;
        return NULL;
    }
    if(index >= self->length){
        indexed_mem = (char*)self->base + (self->item_size * (self->length-1)); //WHEN THE INDEX IS LARGER THAN WHAT IS ALLOWED
    } else {
        indexed_mem = (char*)self->base + (self->item_size * index);
    }
    memcpy(ret, indexed_mem, self->item_size);

    sem_wait(&self->readers);
    self->num_readers--;
    if(self->num_readers == 0){
        sem_post(&self->semaphore);
    }
    sem_post(&self->readers);

    return ret;
}

//THIS IS A WRITER FUNCTION
bool remove_data_al(arraylist_t *self, void *data){
    bool ret = false;
    int deletetion_index;
    sem_wait(&(self->semaphore));
    if(self->length == 0 || self->freed || self->foreach_loopers > 0){
        sem_post(&(self->semaphore));
        return false;
    }
    sem_post(&(self->semaphore));

    //Setting the memory to 0 for deleting
    if(data == NULL){
        deletetion_index = 0;
        sem_wait(&(self->semaphore));
        memset(self->base, 0x0, self->item_size);
        self->length--;
        sem_post(&(self->semaphore));
        ret = true;
    } else {
        sem_wait(&(self->semaphore));
        for(int i = 0; i < self->length; i++){
            void* indexed_mem = (char*)self->base + (self->item_size * i);
            if(memcmp(data, indexed_mem, self->item_size) == 0){
                deletetion_index = i;
                memset(indexed_mem, 0x0, self->item_size);
                self->length--;
                ret = true;
            }
            break;
        }
        sem_post(&(self->semaphore));
    }

    if(self->length == (self->capacity/2) - 1){
        sem_wait(&(self->semaphore));
        resize_al(self);
        sem_post(&(self->semaphore));
    }

    //Only adjust if we found the data
    if(ret == true){
        sem_wait(&(self->semaphore));
        adjust_al(self, deletetion_index);
        sem_post(&(self->semaphore));
    }

    return ret;
}

//THIS IS A WRITER FUNCTION
void *remove_index_al(arraylist_t *self, size_t index){
    void *ret = 0;
    int deletion_index;
    void* indexed_mem;
    ret = malloc(sizeof(self->item_size));

    sem_wait(&(self->semaphore));
    if(self->length == 0 || (int)index < 0 || self->freed || self->foreach_loopers > 0){
        errno = EINVAL;
        sem_post(&(self->semaphore));
        return NULL;
    }
    if((int)index >= self->length){
        indexed_mem = (char*)self->base + (self->item_size * (self->length - 1));
        deletion_index = self->length-1;
    } else {
        indexed_mem = (char*)self->base + (self->item_size * index);
        deletion_index = index;
    }
    memcpy(ret, indexed_mem, self->item_size);
    memset(indexed_mem, 0x0, self->item_size);
    self->length--;
    sem_post(&(self->semaphore));

    //I still need to account for adjusting and  resizing
    if(self->length == (self->capacity/2) - 1){
        sem_wait(&(self->semaphore));
        resize_al(self);
        sem_post(&(self->semaphore));
    }

    sem_wait(&(self->semaphore));
    adjust_al(self, deletion_index);
    sem_post(&(self->semaphore));
    return ret;
}

void delete_al(arraylist_t *self, void (*free_item_func)(void*)){
    //Call free on every item in the arraylist
    if(free_item_func != NULL){
        void* current_item;
        sem_wait(&(self->semaphore));
        for(int i = 0; i < self->length; i++){
            current_item = (char*)self->base + (self->item_size * i);
            free_item_func(current_item);
        }
        sem_post(&(self->semaphore));
    }
    //ELSE do nothing for each item in the arraylist
    sem_wait(&(self->semaphore));
    free(self->base);
    self->freed = true;
    sem_post(&(self->semaphore));
    return;
}

void adjust_al(arraylist_t *self, int starting_index){
    void* indexed_mem;
    void* mem_to_copy;
    for(int i = starting_index; i < self->length; i++){
        //Target location
        indexed_mem = (char*)self->base + (self->item_size * i);
        //Memory to be copied
        mem_to_copy = (char*)indexed_mem + self->item_size;
        memcpy(indexed_mem, mem_to_copy, self->item_size);
    }
}
