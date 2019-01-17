#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <stdio.h>
#include <stdbool.h>
#include "arraylist.h"
#include "foreach.h"
#include <unistd.h>

/******************************************
 *                  ITEMS                 *
 ******************************************/
arraylist_t *global_list;

typedef struct {
    char* name;
    int32_t id;
    double gpa;
}student_t;

typedef struct{
    int i;
    float f;
    long double ld;
    char c1:4;
    char c2:4;
    short s;
    void *some_data;
}test_item_t;

/******************************************
 *              HELPER FUNCS              *
 ******************************************/
void test_item_t_free_func(void *argptr){
    test_item_t* ptr = (test_item_t*) argptr;
    if(!ptr)
        free(ptr->some_data);
    else
        cr_log_warn("%s\n", "Pointer was NULL");
}

void setup(void) {
    cr_log_warn("Setting up test");
    global_list = new_al(sizeof(test_item_t));
}

void teardown(void) {
    cr_log_error("Tearing down");
    delete_al(global_list, test_item_t_free_func);
}

/******************************************
 *                  TESTS                 *
 ******************************************/
Test(al_suite, 0_creation, .timeout=2){
    arraylist_t *locallist = new_al(sizeof(test_item_t));

    cr_assert_not_null(locallist, "List returned was NULL");
}

Test(al_suite, 1_deletion, .timeout=2){
    arraylist_t *locallist = new_al(sizeof(test_item_t));

    cr_assert_not_null(locallist, "List returned was NULL");

    delete_al(locallist, test_item_t_free_func);

    cr_assert(true, "Delete completed without crashing");
}

/*Test(al_suite, 2_insertion, .timeout=2, .init=setup, .fini=teardown){
    cr_assert(true, "I win");
}

Test(al_suite, 3_removal, .timeout=2, .init=setup, .fini=teardown){
}*/

/********************************************************
*                   USER DEFINED TESTS                  *
********************************************************/

Test(basic_al_instantion, 0_creation){
    arraylist_t *int_list = new_al(sizeof(int));
    cr_assert_not_null(int_list, "List returned was NULL");
    cr_assert(int_list->capacity == INIT_SZ, "Capacity is not set to INIT_SZ");
    cr_assert(int_list->length == 0, "Length is not 0");
    cr_assert(int_list->item_size == sizeof(int), "Item size is not size of an int");
    cr_assert_not_null(int_list->base, "The base address of the arraylist is null");
}

Test(insertion_al, 0_insertion){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    insert_al(int_list, &i);
    cr_assert(memcmp((char*)int_list->base, &i, sizeof(int)) == 0, "Inserted memory is not the same");
    cr_assert(int_list->length == 1, "Length is not 1");
}

Test(insertion_al, 1_insertion){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int j = 2;
    int k = 3;
    insert_al(int_list, &i);
    insert_al(int_list, &j);
    insert_al(int_list, &k);
    cr_assert(memcmp((char*)int_list->base, &i, sizeof(int)) == 0, "Inserted memory is not the same");
    cr_assert(memcmp((char*)int_list->base + 4, &j, sizeof(int)) == 0, "Inserted memory is not the same");
    cr_assert(memcmp((char*)int_list->base + 8, &k, sizeof(int)) == 0, "Inserted memory is not the same");
    cr_assert(int_list->length == 3, "Length is not 3");
}

Test(get_data_al, 0_get_data){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int y = 1;
    int z = 2;
    insert_al(int_list, &i);
    int index = get_data_al(int_list, &y);
    int second_index = get_data_al(int_list, &z);
    cr_assert(index == 0, "Index is not 0");
    cr_assert(second_index == UINT_MAX, "Index is not UINT_MAX");
}

Test(get_data_al, 1_get_data){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int y = 10;
    int z = 23;
    insert_al(int_list, &z);
    insert_al(int_list, &i);
    int index = get_data_al(int_list, &y);
    int second_index = get_data_al(int_list, NULL);
    cr_assert(index == UINT_MAX, "Index is not UINT_MAX");
    cr_assert(second_index == 0, "Index is not 0");
}

Test(get_index_al, 0_get_index){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    insert_al(int_list, &i);
    int* item1 = get_index_al(int_list, 0);
    int* item2 = get_index_al(int_list, -1);
    int* item3 = get_index_al(int_list, 1);
    cr_assert(*item1 == 1, "Returned data is not correct");
    cr_assert(*item3 == 1, "Returned data is not correct");
    cr_assert_null(item2, "Returned non-null data pointer");
}

Test(get_index_al, 1_get_index){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int j = 2;
    int k = 3;
    insert_al(int_list, &i);
    insert_al(int_list, &j);
    insert_al(int_list, &k);
    int* item1 = get_index_al(int_list, 1);
    int* item2 = get_index_al(int_list, 10);
    cr_assert(*item1 == 2, "Returned data is not correct");
    cr_assert(*item2 == 3, "Returned data is not correct");
}

Test(resize_al, 0_resize){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    cr_assert(int_list->length == 5, "Length is not 4");
    cr_assert(int_list->capacity == INIT_SZ*2, "List did not resize correctly");
}

Test(resize_al, 1_resize){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    insert_al(int_list, &i);
    remove_data_al(int_list, &i);
    remove_data_al(int_list, &i);
    cr_assert(int_list->capacity == INIT_SZ, "List did not resize correctly");
}

Test(remove_data_al, 0_remove_data){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    insert_al(int_list, &i);
    int x = 1;
    remove_data_al(int_list, &x);
    cr_assert(int_list->length == 0, "Removal failed");
    cr_assert_null(get_index_al(int_list, 0), "Removal failed");
}

Test(remove_data_al, 1_remove_data){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int x = 2;
    insert_al(int_list, &i);
    insert_al(int_list, &x);
    remove_data_al(int_list, NULL);
    int* item1 = get_index_al(int_list, 0);
    cr_assert(int_list->length == 1, "Removal failed");
    cr_assert(*item1 == 2, "Reconfiguration failed");
}

Test(remove_index_al, 0_remove_index){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    insert_al(int_list, &i);
    remove_index_al(int_list, 0);
    cr_assert(int_list->length == 0, "Removal failed");
    cr_assert_null(get_index_al(int_list, 0), "Removal failed");
}

Test(remove_index_al, 1_remove_index){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int x = 2;
    int y = 4;
    insert_al(int_list, &i);
    insert_al(int_list, &x);
    insert_al(int_list, &y);
    remove_index_al(int_list, 1);
    int* item1 = get_index_al(int_list, 1);
    cr_assert(int_list->length == 2, "Removal failed");
    cr_assert(*item1 == 4, "Reconfiguration failed");
}

Test(remove_index_al, 2_remove_index){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int x = 2;
    int y = 4;
    insert_al(int_list, &i);
    insert_al(int_list, &x);
    insert_al(int_list, &y);
    remove_index_al(int_list, 5);
    int* item1 = get_index_al(int_list, 1);
    cr_assert(int_list->length == 2, "Removal failed");
    cr_assert(*item1 == 2, "Reconfiguration failed");
}

Test(add_remove, 0_test){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int x = 2;
    int y = 3;
    insert_al(int_list, &i);
    insert_al(int_list, &x);
    insert_al(int_list, &y);
    insert_al(int_list, &i);
    insert_al(int_list, &x);
    remove_index_al(int_list, 2);
    int* item1 = get_index_al(int_list, 3);
    cr_assert(int_list->length == 4, "Removal failed");
    cr_assert(int_list->capacity == INIT_SZ*2, "Resizing failed");
    cr_assert(*item1 == 2, "Reconfiguration failed");
    remove_index_al(int_list, 0);
    item1 = get_index_al(int_list, 0);
    cr_assert(*item1 == 2, "Reconfiguration failed");
    cr_assert(int_list->capacity == INIT_SZ, "Resizing failed");
}

/******************************************************
                FOREACH TESTING
******************************************************/
int numbers(void* data){
    int *item = data;
    *item = *item + 1;
    return 0;
}

Test(foreach_init, 0_init){
    arraylist_t *int_list = new_al(sizeof(int));
    int i = 1;
    int x = 2;
    int y = 3;
    insert_al(int_list, &i);
    insert_al(int_list, &x);
    insert_al(int_list, &y);
    apply(int_list, numbers);
    int *item = get_index_al(int_list, 0);
    cr_assert(*item == 2, "Value is not 2");
    item = get_index_al(int_list, 1);
    cr_assert(*item == 3, "Value is not 3");
    item = get_index_al(int_list, 2);
    cr_assert(*item == 4, "Value is not 4");
    size_t index = foreach_index();
    cr_assert(index == 2, "Index is not 2");
}

Test(foreach_init, 1_init){
    arraylist_t *int_list = new_al(sizeof(int));
    foreach(void, list_item, int_list){
        cr_assert_fail("We were not supposed to execute this block");
    }
}

Test(foreach_init, 2_init){
    arraylist_t *int_list = new_al(sizeof(int));
    int x = 10;
    insert_al(int_list, &x);
    insert_al(int_list, &x);
    insert_al(int_list, &x);
    foreach(void, list_item, int_list){
        remove_index_al(int_list, 0);
        remove_data_al(int_list, &x);
        cr_assert(int_list->foreach_loopers == 1, "Loopers value was not incremented");
    }

    cr_assert(int_list->foreach_loopers == 0, "Loopers value was not decremented");
    cr_assert(int_list->length == 3, "Length and list items were not preserved");
}

Test(foreach, 0_index){
    arraylist_t *int_list = new_al(sizeof(int));
    int x = 1;
    int y = 2;
    int z = 3;
    int a = 4;
    insert_al(int_list, &x);
    insert_al(int_list, &y);
    insert_al(int_list, &z);
    insert_al(int_list, &a);
    foreach(int, list_item, int_list){
        if(*list_item%2 == 0){
            cr_assert((foreach_index()+1)%2 == 0, "Index is not an even number");
        }
    }
}

Test(foreach, 0_break){
    arraylist_t *int_list = new_al(sizeof(int));
    size_t index;
    int x = 1;
    int y = 2;
    int z = 3;
    int a = 4;
    insert_al(int_list, &x);
    insert_al(int_list, &y);
    insert_al(int_list, &z);
    insert_al(int_list, &a);
    foreach(int, list_item, int_list){
        if(*list_item == 3){
            index = foreach_index();
            foreach_break;
        }
    }
    cr_assert(index == 2, "Index is not 2 after breaking");
}

/***********************************************************
                Arraylist testing with threads
***********************************************************/
void* protocol_1(void* data){
    int x = 1;
    arraylist_t *list = (arraylist_t*)data;
    for(int i = 0; i < 10; i++){
        insert_al(list, &x);
        size_t array_size = list->length;
        printf("Array size is: %d\n", (int)array_size);
        sleep(3);
    }
    pthread_exit(NULL);
}

void* protocol_2(void* data){
    //int x = 1;
    arraylist_t *list = (arraylist_t*)data;
    for(int i = 0; i < 10; i++){
        remove_index_al(list, 0);
        //printf("Capacity is: %d\n", (int)list->capacity);
        printf("We removed\n");
        sleep(2);
    }
    pthread_exit(NULL);
}

Test(arraylist_threading, 0_test){
    pthread_t pid_1;
    pthread_t pid_2;
    pthread_t pid_3;
    arraylist_t *int_list = new_al(sizeof(int));
    for(int i = 0; i < 10; i++){
        insert_al(int_list, &i);
    }
    sleep(2);
    pthread_create(&pid_1, NULL, protocol_1, int_list);
    pthread_create(&pid_2, NULL, protocol_2, int_list);
    pthread_create(&pid_3, NULL, protocol_1, int_list);
    pthread_join(pid_1, NULL);
    pthread_join(pid_2, NULL);
    pthread_join(pid_3, NULL);
    size_t array_size = int_list->length;
    //printf("Array size is actually %d\n", (int)array_size);
    cr_assert(array_size == 20, "Array size is not 20");
    cr_assert(int_list->capacity == 32, "Capacity is not 32");
}

void* test1_protocol1(void* data){
    arraylist_t *list = data;
    for(int i = 1; i < 4; i++){
        remove_index_al(list, i);
        sleep(2);
    }
    pthread_exit(NULL);
}

void* test1_protocol2(void* data){
    arraylist_t *list = data;
    for(int i = 0; i < 5; i++){
        insert_al(list, &i);
        sleep(1);
    }
    pthread_exit(NULL);
}

Test(arraylist_threading, 1_test){
    pthread_t pid_1;
    pthread_t pid_2;
    arraylist_t *int_list = new_al(sizeof(int));
    for(int i = 0; i < 6; i++){
        insert_al(int_list, &i);
    }
    pthread_create(&pid_1, NULL, test1_protocol1, int_list);
    pthread_create(&pid_2, NULL, test1_protocol2, int_list);
    pthread_join(pid_1, NULL);
    pthread_join(pid_2, NULL);
    int numbers_array[] = {0,2,4,0,1,2,3,4};
    int memory_compare = memcmp(int_list->base, numbers_array, sizeof(int)*8);
    cr_assert(int_list->length == 8, "Length is not 8");
    cr_assert(memory_compare == 0, "The memory blocks are not the same");
}

void* test2_protocol_delete(void* data){
    arraylist_t *list = data;
    for(int i = 0; i < 5; i++){
        sleep(2);
        if(i == 4){
            delete_al(list, NULL);
        }
    }
    pthread_exit(NULL);
}

Test(arraylist_threading, 2_test_delete){
    pthread_t pid_1;
    pthread_t pid_2;
    pthread_t pid_3;
    int x = 5;
    arraylist_t *int_list = new_al(sizeof(int));
    for(int i = 0; i < 6; i++){
        insert_al(int_list, &i);
    }
    pthread_create(&pid_1, NULL, test1_protocol1, int_list);
    pthread_create(&pid_2, NULL, test1_protocol2, int_list);
    pthread_create(&pid_3, NULL, test2_protocol_delete, int_list);
    pthread_join(pid_1, NULL);
    pthread_join(pid_2, NULL);
    pthread_join(pid_3, NULL);
    cr_assert(int_list->freed == true, "Arraylist was not freed");
    cr_assert(insert_al(int_list, &x) == UINT_MAX, "Error case was not triggered");
    cr_assert(get_data_al(int_list, &x) == UINT_MAX, "Error case was not triggered");
    cr_assert(get_index_al(int_list, 0) == NULL, "Error case was not trigggered");
    cr_assert(remove_index_al(int_list, 0) == NULL, "Error case was not triggered");
    cr_assert(remove_data_al(int_list, &x) == false, "Error case was not triggered");
}

void* foreach_looper_thread(void* data){
    arraylist_t *int_list = data;
    foreach(void, int_item, int_list){
        sleep(1);
        remove_index_al(int_list, 0);
    }
    pthread_exit(NULL);
}

void* foreach_looper_insert(void *data){
    arraylist_t *int_list = data;
    int x = 2;
    for(int i = 0; i < 100; i++){
        insert_al(int_list, &x);
    }
    pthread_exit(NULL);
}

Test(foreach_threading, 3_test){
    pthread_t pid_1;
    pthread_t pid_2;
    int x = 1;
    arraylist_t *int_list = new_al(sizeof(int));
    for(int i = 0; i < 100; i++){
        insert_al(int_list, &x);
    }
    //Foreach looper thread
    pthread_create(&pid_1, NULL, foreach_looper_thread, int_list);
    pthread_create(&pid_2, NULL, foreach_looper_insert, int_list);
    pthread_join(pid_1, NULL);
    pthread_join(pid_2, NULL);
    printf("The list length is: %d\n", (int)int_list->length);
    cr_assert(int_list->length == 200, "Length is not 200");
}

/************************************************************************
                        Many thread testing
************************************************************************/
void* insert_protocol(void* data){
    arraylist_t *list = data;
    int x = 10;
    insert_al(list, &x);
    pthread_exit(NULL);
}

void* remove_protocol(void* data){
    arraylist_t *list = data;
    remove_index_al(list, 0);
    pthread_exit(NULL);
}

Test(many_thread, 0_test){
    pthread_t pid1;
    pthread_t pid2;
    arraylist_t *int_list1 = new_al(sizeof(int));
    arraylist_t *int_list2 = new_al(sizeof(int));
    for(int i = 0; i < 100; i++){
        insert_al(int_list2, &i);
    }
    for(int i = 0; i < 100; i++){
        pthread_create(&pid1, NULL, insert_protocol, int_list1);
        pthread_create(&pid2, NULL, remove_protocol, int_list2);
    }
    cr_assert(int_list1->length == 100, "List is not of length 100");
    cr_assert(int_list2->length == 0, "List is not of length 0");
}

/*int main(int arc, char** argv){
    arraylist_t *int_list = new_al(sizeof(int));
    int x = 10;
    insert_al(int_list, &x);
    insert_al(int_list, &x);
    insert_al(int_list, &x);
    foreach(void, list_item, int_list){
        remove_index_al(int_list, 0);
        remove_data_al(int_list, &x);
        if(int_list->foreach_loopers == 1)
            printf("Loopers value was not incremented");
    }

    if(int_list->foreach_loopers == 0)
        printf("Loopers value was not decremented");
    if(int_list->length == 3)
        printf("Length and list items were not preserved");
    return 0;
}*/
