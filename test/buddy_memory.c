#include <stdio.h>
#include <stdlib.h>

#define MAX_ORDER 4
#define TOT_SIZE 1024
#define MIN_SIZE 64

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
		// void *rem = available_list[j].head;
		available_list[j].head = *(void **) (available_list[j].head + 2 * sizeof(int)); // head = head->next
		*(void **) (available_block + 2 * sizeof(int)) = NULL; // rem->next = NULL
		*(void **) (available_list[j].head + 2 * sizeof(int) + sizeof(void *)) = NULL; // head->back = NULL
	}

	printf("The block with address: %p is removed from available list index: %d\n", available_block, j);

	while (j != order){ // need to be splitted
		j--; // decrease order
		void *splitted_add = available_block + (MIN_SIZE * (1 << j)); // next block

		// // tag - available bit
		// void *tmp_add = splitted_add;
		// int *tag = (int *) tmp_add;
		// *tag = 1;
		// tmp_add += sizeof(int);
		//
		// // order of the block
		// int *k = (int *) tmp_add;
		// *k = j;
		// tmp_add += sizeof(int);
		//
		// // next block pointer
		// void **next = (void**) tmp_add;
		// *next = NULL;
		// tmp_add += sizeof(void *);
		//
		// // back block pointer
		// void **back = (void**) tmp_add;
		// *back = NULL;
		// tmp_add += sizeof(void *);

		*(int *) (splitted_add) = 1;
		*(int *) (splitted_add + sizeof(int)) = j;
		*(void **) (splitted_add + 2*sizeof(int)) = NULL;
		*(void **) (splitted_add + 2*sizeof(int) + sizeof(void *)) = NULL;

		// add splitted and available block to the available list -- sorted
		// if head is null, new head becomes splitted address
	  if (available_list[j].head == NULL && available_list[j].tail == NULL) { //empty
	    	available_list[j].head = splitted_add;
				available_list[j].tail = splitted_add;
		}
	    else {
				void *next_block;
				for (next_block = available_list[j].head; next_block != NULL && splitted_add > next_block; next_block = *(void **)(next_block + 2 * sizeof(int)));
				if (next_block == NULL){ // next to the tail
					*(void **)(available_list[j].tail + 2 * sizeof(int)) = splitted_add; // tail->next = splitted
					*(void **) (splitted_add + 2*sizeof(int) + sizeof(void *)) = available_list[j].tail; //splitted->back = tail
					available_list[j].tail = splitted_add; //tail = splitted
				}
				else if (next_block == available_list[j].head){
					*(void **) (splitted_add + 2*sizeof(int)) = available_list[j].head; // splitted->next = head
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

	return available_block + 2 * sizeof(int);
}

void deallocate(void *list_add, void *ptr){
	void *block_add = ptr - 2*sizeof(int);

	// finding buddy add
	int order_of_block = *(int *) (block_add + sizeof(int));
	int size_block = MIN_SIZE * (1 << order_of_block);
	int buddy_num = (block_add - list_add) / size_block;
	void *buddy_add;
	if (buddy_num % 2 == 0)
		buddy_add = block_add + size_block;
	else
		buddy_add = block_add - size_block;

	// can buddy and block be merged
	int buddy_order = *(int *) (buddy_add + sizeof(int));
	int buddy_tag = *(int *) (buddy_add + sizeof(int));
	if (buddy_order == order_of_block && buddy_tag == 1){ // can be merged
			// merge add
			void *merge_add;
			if (buddy_num % 2 == 0)
				merge_add = block_add;
			else
				merge_add = buddy_add;

			// remove buddy block from available list
			if (buddy_add == available_list[buddy_order].head){
				if (available_list[buddy_order].tail == available_list[buddy_order].head){
					available_list[buddy_order].tail = NULL;
					available_list[buddy_order].head = NULL;
				}
				else{
					// void *rem = available_list[buddy_order].head;
					available_list[buddy_order].head = *(void **) (available_list[buddy_order].head + 2 * sizeof(int)); // head = head->next
					*(void **) (buddy_add + 2 * sizeof(int)) = NULL; // buddy->next = NULL
					*(void **) (available_list[buddy_order].head + 2 * sizeof(int) + sizeof(void *)) = NULL; // head->back = NULL
				}
			}
			else if (buddy_add == available_list[buddy_order].tail){
				if (available_list[buddy_order].tail == available_list[buddy_order].head){
					available_list[buddy_order].tail = NULL;
					available_list[buddy_order].head = NULL;
				}
				else{
					// void *rem = available_list[buddy_order].tail;
					available_list[buddy_order].tail = *(void **) (available_list[buddy_order].tail + 2 * sizeof(int) + sizeof(void *)); // tail = tail->prev
					*(void **) (buddy_add + 2 * sizeof(int) + sizeof(void *)) = NULL; // buddy->prev = NULL
					*(void **) (available_list[buddy_order].tail + 2 * sizeof(int)) = NULL; // tail->next = NULL
				}
			}
			else {
				void *buddy_next = *(void **) (buddy_add + 2 * sizeof(int));
				void *buddy_prev = *(void **) (buddy_add + 2 * sizeof(int) + sizeof(void *));

				*(void **) (buddy_prev + 2 * sizeof(int)) = buddy_next; //buddy_prev->next = buddy_next
				*(void **) (buddy_next + 2 * sizeof(int) + sizeof(void *)) = buddy_prev; //buddy_next->prev = buddy_prev
				*(void **) (buddy_add + 2 * sizeof(int)) = NULL; // buddy->next = NULL
				*(void **) (buddy_add + 2 * sizeof(int) + sizeof(void *)) = NULL; // buddy->prev = NULL
			}

			*(int *) (merge_add) = 1;
			*(int *) (merge_add + sizeof(int)) = ++buddy_order;
			*(void **) (merge_add + 2*sizeof(int)) = NULL;
			*(void **) (merge_add + 2*sizeof(int) + sizeof(void *)) = NULL;

			// add merged and available block to the available list -- sorted
			// if head is null, new head becomes splitted address
			if (available_list[buddy_order].head == NULL && available_list[buddy_order].tail == NULL) { //empty
					available_list[buddy_order].head = merge_add;
					available_list[buddy_order].tail = merge_add;
			}
			else {
				void *next_block;
				for (next_block = available_list[buddy_order].head; next_block != NULL && merge_add > next_block; next_block = *(void **)(next_block + 2 * sizeof(int)));
				if (next_block == NULL){ // next to the tail
					*(void **)(available_list[buddy_order].tail + 2 * sizeof(int)) = merge_add; // tail->next = merged
					*(void **) (merge_add + 2*sizeof(int) + sizeof(void *)) = available_list[buddy_order].tail; //merged->back = tail
					available_list[buddy_order].tail = merge_add; //tail = merged
				}
				else if (next_block == available_list[buddy_order].head){
					*(void **) (merge_add + 2*sizeof(int)) = available_list[buddy_order].head; // merged->next = head
					*(void **)(available_list[buddy_order].head + 2 * sizeof(int) + sizeof(void *)) = merge_add; //head->back = merged
					available_list[buddy_order].head = merge_add; // head = merged
				}
				else {
					void *prev = *(void **)(next_block + 2 * sizeof(int) + sizeof(void *)); //prev = next_block->back
					*(void **)(merge_add + 2 * sizeof(int)) = next_block; // merged->next = next_block
					*(void **)(next_block + 2 * sizeof(int) + sizeof(void *)) = merge_add; //next_block->back = merged
					*(void **)(prev + 2 * sizeof(int)) = merge_add; // prev->next = merged
					*(void **)(merge_add + 2 * sizeof(int) + sizeof(void *)) = prev; // merged->back = prev
				}
		 }
		 printf("The block with address: %p is merged and added to available list index: %d\n", merge_add, buddy_order);
		 deallocate(list_add, merge_add);
	}
	else { // cannot merged
		*(int *) (block_add) = 1;
		*(int *) (block_add + sizeof(int)) = order_of_block;
		*(void **) (block_add + 2*sizeof(int)) = NULL;
		*(void **) (block_add + 2*sizeof(int) + sizeof(void *)) = NULL;

		// add merged and available block to the available list -- sorted
		// if head is null, new head becomes splitted address
		if (available_list[order_of_block].head == NULL && available_list[order_of_block].tail == NULL) { //empty
				available_list[order_of_block].head = block_add;
				available_list[order_of_block].tail = block_add;
		}
		else {
			void *next_block;
			for (next_block = available_list[order_of_block].head; next_block != NULL && block_add > next_block; next_block = *(void **)(next_block + 2 * sizeof(int)));
			if (next_block == NULL){ // next to the tail
				*(void **)(available_list[order_of_block].tail + 2 * sizeof(int)) = block_add; // tail->next = merged
				*(void **) (block_add + 2*sizeof(int) + sizeof(void *)) = available_list[order_of_block].tail; //merged->back = tail
				available_list[order_of_block].tail = block_add; //tail = merged
			}
			else if (next_block == available_list[order_of_block].head){
				*(void **) (block_add + 2*sizeof(int)) = available_list[order_of_block].head; // merged->next = head
				*(void **)(available_list[order_of_block].head + 2 * sizeof(int) + sizeof(void *)) = block_add; //head->back = merged
				available_list[order_of_block].head = block_add; // head = merged
			}
			else {
				void *prev = *(void **)(next_block + 2 * sizeof(int) + sizeof(void *)); //prev = next_block->back
				*(void **)(block_add + 2 * sizeof(int)) = next_block; // merged->next = next_block
				*(void **)(next_block + 2 * sizeof(int) + sizeof(void *)) = block_add; //next_block->back = merged
				*(void **)(prev + 2 * sizeof(int)) = block_add; // prev->next = merged
				*(void **)(block_add + 2 * sizeof(int) + sizeof(void *)) = prev; // merged->back = prev
			}
	 	}
		printf("The block with address: %p is added to available list index: %d without merging\n", block_add, order_of_block);
	}
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
	// void *tmp_add = list_add;
	// int *tag = (int *) tmp_add;
	// *tag = 1;
	//
	// tmp_add += sizeof(int);
	//
	// // order of the block
	// int *k = (int *) tmp_add;
	// *k = MAX_ORDER;
	// tmp_add += sizeof(int);
	//
	// // next block pointer
	// void **next = (void**) tmp_add;
	// *next = NULL;
	// tmp_add += sizeof(void *);
	//
	// // back block pointer
	// void **back = (void**) tmp_add;
	// *back = NULL;

	*(int *) (list_add) = 1;
	*(int *) (list_add + sizeof(int)) = MAX_ORDER;
	*(void **) (list_add + 2*sizeof(int)) = NULL;
	*(void **) (list_add + 2*sizeof(int) + sizeof(void *)) = NULL;

	void *all_00 = allocate(list_add, 0);
	void *all_10 = allocate(list_add, 1);
	void *all_01 = allocate(list_add, 0);
	void *all_11 = allocate(list_add, 1);
	deallocate(list_add, all_10);
	deallocate(list_add, all_11);
	deallocate(list_add, all_00);
	deallocate(list_add, all_01);
	return 0;
}
