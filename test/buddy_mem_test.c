#include <stdio.h>
#include <stdlib.h>

struct available_node{
  void *head;
  void *tail;
};

// the whole segment size is 8192
// the min req size = 128
struct available_node available_list[7];

void *allocate(void* list_add, int order){
  int j;
  for (j = order; available_list[j].head == NULL; ++j);
  void *available_block = available_list[j].head;
  if (available_list[j].tail == available_list[j].head)
    available_list[j].tail = NULL;
  void ** tmp = (void **) (available_list[j].head + 2*sizeof(int));
  available_list[j].head = *tmp;
  while (j != order){
    j--;
    void *splitted_add = available_block + (128 * (1 << j));
    printf("SPLIT_ADD: %p\n", splitted_add);
    available_list[j].tail = splitted_add;
    if (available_list[j].head == NULL) //empty
      available_list[j].head = splitted_add;
    else{
      tmp = (void **)(available_list[j].tail + 2 * sizeof(int));
      *tmp = splitted_add;
    }
  }

  int *tag = (int *) available_block;
  *tag = 0;

  printf("ADD: %p\n", available_block);
  return available_block + sizeof(int);
}

int main(int argc, char const *argv[]) {
  /* code */
  void *list_add = malloc(8192);

  int index;
  for (index = 0; index < 6; ++index){
    available_list[index].head = NULL;
    available_list[index].tail = NULL;
  }

  available_list[7].head = list_add;
  available_list[7].tail = list_add;

  void *tmp_add = list_add;
  int *tag = (int *) tmp_add;
  *tag = 1;
  tmp_add += sizeof(int);

  int *k = (int *) tmp_add;
  *k = 3;
  tmp_add += sizeof(int);

  void **next = (void**) tmp_add;
  *next = NULL;
  tmp_add += sizeof(void *);
  printf("ADD_START: %p\n", list_add);
  allocate(list_add, 1);
  allocate(list_add, 1);
  allocate(list_add, 2);
  return 0;
}
