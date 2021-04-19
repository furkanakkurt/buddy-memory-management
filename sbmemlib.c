#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.
#define SHAREDMEM_NAME "/shared_memory"

// Define semaphore(s)


// Define your stuctures and variables. 

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

	// to map shared memory into address space of the process
	void *shm_start;

	// return of the 
	int fd;

	// if already a shared segment with the same name
	int ret;

	// if we want to get info about shared memory
	struct stat sbuf;

	// holds the name of the shared memory
	char sharedmem_name[100];
	strcpy(sharedmem_name, SHAREDMEM_NAME);

	// previously opened shared segment
	ret = shm_unlink(sharedmem_name);
	if (ret == 0) {
		printf("Previously opened shared memory segment has been closed.\n");
	}

	// open shared memory 
	fd = shm_open(sharedmem_name, O_RDWR | O_CREAT, 0660);

	if (fd == -1) {
		perror("Error while creating a shared memory segment.\n");
		exit(1);
	}
	else {
		printf("Shared memory segment has been initialized.\n");
	}

	// set the size of the shared memory
	ftruncate(fd, segmentsize);

	// info about shared memory
	fstat(fd, &sbuf);

	// mapping
	//void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
	// addr: starting address, length: length of the mapping > 0
	shm_start = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// file descriptor can be closed
	close(fd);

	printf ("sbmem init called"); // remove all printfs when you are submitting to us.  
    return (0); 
}

int sbmem_remove()
{

	// remove shared memory segment from system
	// use shm_unlin()
	// do necessary cleanups, such as removing semaphores 

    return (0); 
}

int sbmem_open()
{

	// this function indicates to the library that the process would like
	// to use the library
	// That way, processes can be kept track of
	// library will map shared segment to virtual addr. space of process
	// using mmmap()
	// if too many processes using library, return -1, else return 0	

    return (0); 
}


void *sbmem_alloc (int size)
{

	// allocate memory of size n = 2^k >= reqsize
	// on success, return pointer to allocated space, else NULL

    return (NULL);
}


void sbmem_free (void *p)
{

	// deallocate memory that was allocated earlier
 
}

int sbmem_close()
{
    
	// when process is done using the library, this function will be called
	// shared segment will be unmapped from virtual addr. space of process 
	// if process wants to use again, call sbmem_open() again
	// previously allocated space by the process does not need to be deallocated
	// it is OPTIONAL

    return (0); 
}
