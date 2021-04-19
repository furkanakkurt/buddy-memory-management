
#include <stdio.h>
#include <stdlib.h>

// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.

// Define semaphore(s)

// Define your stuctures and variables. 


/*
struct ava {
	void * head;
	void * tail;
}

struct ava * freeList[m+1]; 
freeList[m] =  

*/




int sbmem_init(int segmentsize)
{

	// create and initialize a sharedmem of segmentsize
	// is successful, return 0, else - 1
	// use POSIX shm_open() and ftruncate()
	// if already a shared segment, destroy it first with the same name
	// for destruction, use sbmem_remove() ?? 


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

	

    return (NULL);
}


void sbmem_free (void *p)
{

 
}

int sbmem_close()
{
    
    return (0); 
}
