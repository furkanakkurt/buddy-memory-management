#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.
#define SHAREDMEM_NAME "/shared_memory"

// Define semaphore names
// One semaphore for mutex 
// One semaphore for counting active process
#define SEM_MUTEX "/sem_mutex"
#define SEM_COUNT "/sem_count"

// Define number of active processes allowed
#define MAX_PROCESSES_ALLOWED 10
#define MIN_BLOCK_SIZE 128

// Define semaphore(s)
sem_t *sem_mutex;
sem_t *sem_count;

// Define your stuctures and variables. 
// to keep information about the available blocks 
struct available_node {
	long head;
	long tail;
};

void *process_add;

// You will use semaphore(s) in your library to protect your shared structures. 
// Processes may call allocation and free functions concurrently. 
// This should not cause race
// conditions. You will ensure that by using semaphores.

// create and initialize a shared memory of segmentsize
// use POSIX shm_open() and ftruncate()
// if already a shared segment, destroy it first with the same name
// is successful, return 0, else - 1
// for destruction, use sbmem_remove() ?? 
int sbmem_init(int segmentsize)
{
	// init semaphores
	sem_mutex = sem_open(SEM_MUTEX, O_RDWR | O_CREAT, 0660, 1);
	if (sem_mutex < 0) {
		perror("can not create semaphore\n");
		exit(1); 
	}

	sem_count = sem_open(SEM_MUTEX, O_RDWR | O_CREAT, 0660, 1);
	if (sem_count < 0) {
		perror("can not create semaphore\n");
		exit(1); 
	}

	// get the order
	int order = (int) (log(segmentsize / MIN_BLOCK_SIZE) / log(2));

	// total size
	int total_size = segmentsize + ((order + 1) * sizeof(available_node)) + 2*sizeof(int); // segment + available_list + count + order

	// to map shared memory into address space of the process
	void *shm_start;

	// file descriptor 
	int fd;

	// if already a shared segment with the same name
	int ret;

	// if we want to get info about shared memory
	struct stat sbuf;

	// previously opened shared segment
	ret = shm_unlink(SHAREDMEM_NAME);
	if (ret == 0) {
		printf("Previously opened shared memory segment has been closed.\n");
	}

	// open shared memory 
	fd = shm_open(SHAREDMEM_NAME, O_RDWR | O_CREAT, 0660);

	if (fd == -1) {
		// perror("Error while creating a shared memory segment.\n");
		return -1;
	}
	else {
		printf("Shared memory segment has been initialized.\n");
	}

	// set the size of the shared memory
	ftruncate(fd, total_size);

	// info about shared memory
	fstat(fd, &sbuf);

	// mapping
	//void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
	// addr: starting address, length: length of the mapping > 0
	shm_start = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// file descriptor can be closed
	close(fd);

	*(int *) (shm_start) = 0; // count
	*(int *) (shm_start + sizeof(int)) = order;

	// init available_list
	void *tmp_add = shm_start + 2 * sizeof(int);
	for (int i = 0; i <= order; ++i){
		struct available_node av;
		if (i != order){
			av.head = -1;
			av.tail = -1;
		}
		else {
			av.head = 0; // distance
			av.tail = 0;	
		}
		*(struct available_node *) (tmp_add) = av;
		tmp_add += sizeof(struct available_node);	
	}

	*(int *) (tmp_add) = 1; //tag
	*(int *) (tmp_add + sizeof(int)) = order; //order
	*(long *) (tmp_add) = -1; //distance next
	*(long *) (tmp_add) = -1; //distance prev

	printf ("sbmem init called"); // remove all printfs when you are submitting to us.  
    
    return 0; 
}

int sbmem_remove()
{
	// return from shm_unlink()
	int ret;

	ret = shm_unlink(SHAREDMEM_NAME);

	if (ret == 0) {
		printf("Shared segment is removed.\n");
		return 0;
	} 
	else {
		// perror("Error while removing the shared segment.\n");
		return -1;
	}

	// remove semaphores
	sem_close(sem_mutex);
	sem_close(sem_count);

    return 0; 
}

int sbmem_open()
{
	int fd;

	// open shared memory 
	fd = shm_open(SHAREDMEM_NAME, O_RDWR, 0660);

	if (fd == -1) {
		perror("Error while creating a shared memory segment.\n");
		return -1;
	}
	else {
		printf("Shared memory segment has been initialized.\n");
	}


	process_add = mmap(NULL, NULL, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	close(fd);

	sem_wait(sem_count);
	int count = *(int *) (process_add);
	if (count >= 10)
		return -1;
	(*(int *) process_add)++;
	sem_post(sem_count);	

    return (0); 
}


void *sbmem_alloc (int size)
{

	// allocate memory of size n = 2^k >= reqsize
	// on success, return pointer to allocated space, else NULL

	// finding available order
	int req_size = size + 2 * sizeof(int);
	int order;
	for (order = 0; req_size > (MIN_BLOCK_SIZE * (1 << order)); ++order); //requested order

	int j; // finding the min available order
	int max_order = *(int *) (process_add + sizeof(int));
	for (j = order; available_list[j].head == -1 && j < max_order; ++j);

	void *starting_add = *(void **) (process_add + 2*sizeof(int) + (order + 1) * sizeof(struct available_node));
	void *available_block = starting_add + available_list[j].head;
	if (available_list[j].tail == available_list[j].head){
		available_list[j].tail = -1;
		available_list[j].head = -1;
	}
	else{
		available_list[j].head = *(int *) (starting_add + available_list[j].head + 2 * sizeof(int)); // head = head->next
		*(long *) (available_block + 2 * sizeof(int)) = -1; // rem->next = NULL
		*(long *) (available_list[j].head + 2 * sizeof(int) + sizeof(long)) = -1; // head->back = NULL
	}

	printf("The block with address: %p is removed from available list index: %d\n", available_block, j);

	while (j != order){ // need to be splitted
		j--; // decrease order
		void *splitted_add = available_block + (MIN_BLOCK_SIZE * (1 << j)); // next block

		*(int *) (splitted_add) = 1;
		*(int *) (splitted_add + sizeof(int)) = j;
		*(long *) (splitted_add + 2*sizeof(int)) = -1;
		*(long *) (splitted_add + 2*sizeof(int) + sizeof(long)) = -1;

		// add splitted and available block to the available list -- sorted
		// if head is null, new head becomes splitted address
	  	if (available_list[j].head == -1 && available_list[j].tail == -1) { //empty
	    	available_list[j].head = splitted_add - starting_add;
			available_list[j].tail = splitted_add - starting_add;
		}
    	else {
			long next_block;
			// !!!!!!
			for (next_block = available_list[j].head; next_block != -1 && splitted_add - starting_add > next_block; next_block = *(long *)(next_block + 2 * sizeof(int)));
			// !!!!!!

			if (next_block == -1){ // next to the tail
				*(long *) (available_list[j].tail + 2 * sizeof(int)) = splitted_add - starting_add; // tail->next = splitted
				*(long *) (splitted_add + 2*sizeof(int) + sizeof(long)) = available_list[j].tail; //splitted->back = tail
				available_list[j].tail = splitted_add - starting_add; //tail = splitted
			}
			else if (next_block == available_list[j].head){
				*(long *) (splitted_add + 2*sizeof(int)) = available_list[j].head; // splitted->next = head
				*(long *)(available_list[j].head + 2 * sizeof(int) + sizeof(long)) = splitted_add - starting_add; //head->back = splitted
				available_list[j].head = splitted_add - starting_add; // head = splitted
			}
			else {
				long prev = *(long *)(starting_add + next_block + 2 * sizeof(int) + sizeof(long)); //prev = next_block->back
				*(long *)(splitted_add + 2 * sizeof(int)) = next_block; // splitted->next = next_block
				*(long *)(starting_add + next_block + 2 * sizeof(int) + sizeof(long)) = splitted_add - starting_add; //next_block->back = splitted
				*(long *)(starting_add + prev + 2 * sizeof(int)) = splitted_add - starting_add; // prev->next = splitted
				*(long *)(splitted_add + 2 * sizeof(int) + sizeof(long)) = prev; // splitted->back = prev
			}
   		}	

	 	printf("The block with address: %p is added to available list index: %d\n", splitted_add, j);
	}

	*(int *) available_block = 0;
	*(int *) (available_block + sizeof(int)) = order;

	return available_block + 2 * sizeof(int);
}


void sbmem_free (void *p)
{

	// deallocate memory that was allocated earlier
 
}

int sbmem_close()
{
    
	// to unmap, use munmap(void * address, size_t length)


	// when process is done using the library, this function will be called
	// shared segment will be unmapped from virtual addr. space of process 
	// if process wants to use again, call sbmem_open() again
	// previously allocated space by the process does not need to be deallocated
	// it is OPTIONAL

    return (0); 
}
