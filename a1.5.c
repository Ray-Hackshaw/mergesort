/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Ray Hackshaw  
    UPI: rhac635  

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h>

#define SIZE    2

long num_of_threads = 0;
long number_of_processors;
pthread_spinlock_t s_lock;

struct block {
    int size;
    int *first;
};

void print_block_data(struct block *blk) {
    printf("size: %d address: %p\n", blk->size, blk->first);
}

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int combined[left->size + right->size];
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
}

/* Merge sort the data. */
void *merge_sort(void* my_data) {
    struct block *my_data2 = (struct block *) my_data;
    if (my_data2->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data2->size / 2;
        left_block.first = my_data2->first;
        right_block.size = left_block.size + (my_data2->size % 2);
        right_block.first = my_data2->first + left_block.size;

        pthread_t myThread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 1000000000);
        
        if (num_of_threads < number_of_processors)
            {
            pthread_spin_lock(&s_lock);
            num_of_threads++;
            pthread_spin_unlock(&s_lock);
    
            pthread_create(&myThread, &attr, merge_sort, &left_block);
            merge_sort(&right_block);
            pthread_join(myThread,NULL);

            pthread_spin_lock(&s_lock);
            num_of_threads--;
            pthread_spin_unlock(&s_lock);
            } 
        else{
            merge_sort(&left_block);
            merge_sort(&right_block);
            }
        merge(&left_block, &right_block);
        pthread_spin_destroy(&s_lock);
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

int main(int argc, char *argv[]) {

    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    limit.rlim_cur=1000000000;
    limit.rlim_max=1000000000;
    setrlimit(RLIMIT_STACK,&limit);

	long size;
    pthread_spin_init(&s_lock, PTHREAD_PROCESS_PRIVATE);
    number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
    struct block start_block;
    int data[size];
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }
    printf("starting---\n");
    merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}