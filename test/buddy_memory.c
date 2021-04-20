#include <stdio.h>
#include <stdlib.h>

#define MAX_ORDER 6
#define TOT_SIZE 8192
#define MIN_SIZE 128

struct available_node{
	void *head;
	void *tail;
};

struct available_node available_list[MAX_ORDER + 1];


/* allocates memory of size 2^order */
// TODO constrains will be added
// instead of order - reqsize needed
void *allocate(void* list_add, int order){

	int j;

	// finding the minimum available order
	for (j = order; available_list[j].head == NULL && j < MAX_ORDER; ++j);

	// remove head from the linked list for specific order
	void *available_block = available_list[j].head;
	if (available_list[j].tail == available_list[j].head){
		available_list[j].tail = NULL;
		available_list[j].head = NULL;
	}
	else{
		void *rem = available_list[j].head;
		available_list[j].head = *(void **) (available_list[j].head + 2 * sizeof(int)); // head = head->next
		*(void **) (rem + 2 * sizeof(int)) = NULL; // rem->next = NULL
		*(void **) (available_list[j].head + 2 * sizeof(int) + sizeof(void *)) = NULL; // head->back = NULL
	}

	printf("The block with address: %p is removed from available list index: %d\n", available_block, j);

	while (j != order){ // need to be splitted
		j--; // decrease order
		void *splitted_add = available_block + (MIN_SIZE * (1 << j)); // next block

		// tag - available bit
		void *tmp_add = splitted_add;
		int *tag = (int *) tmp_add;
		*tag = 1;
		tmp_add += sizeof(int);

		// order of the block
		int *k = (int *) tmp_add;
		*k = j;
		tmp_add += sizeof(int);

		// next block pointer
		void **next = (void**) tmp_add;
		*next = NULL;
		tmp_add += sizeof(void *);

		// back block pointer
		void **back = (void**) tmp_add;
		*back = NULL;
		tmp_add += sizeof(void *);


		// add splitted and available block to the available list -- sorted
		// if head is null, new head becomes splitted address
	  if (available_list[j].head == NULL && available_list[j].tail == NULL) { //empty
	    	available_list[j].head = splitted_add;
				available_list[j].tail = splitted_add;
		}
	    else {	// tail->next
				void *next_block;
				for (next_block = available_list[j].head; next_block != NULL && splitted_add > next_block; next_block = *(void **)(next_block + 2 * sizeof(int)));
				if (next_block == NULL){ // next to the tail
					*(void **)(available_list[j].tail + 2 * sizeof(int)) = splitted_add; // tail->next = splitted
					back = available_list[j].tail; //splitted->back = tail
					available_list[j].tail = splitted_add; //tail = splitted
				}
				else if (next_block == available_list[j].head){
					next = available_list[j].head; // splitted->next = head
					*(void **)(available_list[j].head + 2 * sizeof(int) + sizeof(void *)) = splitted_add; //head->back = splitted
					available_list[j].head = splitted_add; // head = splitted
				}
				else {
					void *prev = *(void **)(next_block + 2 * sizeof(int) + sizeof(void *)); //prev = next_block->back
					*(void **)(splitted_add + 2 * sizeof(int)) = next_block; // splitted->next = next_block
					*(void **)(next_block + 2 * sizeof(int) + sizeof(void *)) = splitted_add; //next_block->back = splitted
					*(void **)(prev + 2 * sizeof(int)) = splitted_add; // prev->next = splitted
					*(void **)(splitted_add + 2 * sizeof(int) + sizeof(void *)) = prev; // splitted->back = prev
				}
	   }

		 printf("The block with address: %p is added to available list index: %d\n", splitted_add, j);
	}

	int *tag = (int *) available_block;
	*tag = 0;

	int *k = (int *) (available_block + sizeof(int));
	*k = order;

	return available_block + sizeof(int);
}

void deallocate(void *list_add, void *ptr){
	// is ptr block has a buddy

}

int main(int argc, char const *argv[]) {
  /* code */
	void *list_add = malloc(TOT_SIZE);
	// initialize
	int index;
	for (index = 0; index < MAX_ORDER - 1; ++index){
		available_list[index].head = NULL;
		available_list[index].tail = NULL;
	}

	available_list[MAX_ORDER].head = list_add;
	available_list[MAX_ORDER].tail = list_add;

	// tag - available bit
	void *tmp_add = list_add;
	int *tag = (int *) tmp_add;
	*tag = 1;
	tmp_add += sizeof(int);

	// order of the block
	int *k = (int *) tmp_add;
	*k = MAX_ORDER;
	tmp_add += sizeof(int);

	// next block pointer
	void **next = (void**) tmp_add;
	*next = NULL;
	tmp_add += sizeof(void *);

	// back block pointer
	void **back = (void**) tmp_add;
	*back = NULL;

	allocate(list_add, 1);
	allocate(list_add, 0);
	allocate(list_add, 1);
	allocate(list_add, 0);
	return 0;
}
