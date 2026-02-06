#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//header
struct heapchunk_t{
	uint32_t size;
	bool inuse;
	struct heapchunk_t *next;
};

struct heapinfo_t{
	struct heapchunk_t *start;
	uint32_t avail;
};

//global pointer to track the start of the heap
struct heapinfo_t my_heap;

// initialize heap
void init_heap(){
	size_t page_size = getpagesize();
	void *ptr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED){
		perror("mmap failed");
		return;
	}
	my_heap.start = (struct heapchunk_t *)ptr;
	my_heap.avail = page_size -sizeof(struct heapchunk_t);

	my_heap.start -> size = page_size - sizeof(struct heapchunk_t);
	my_heap.start -> inuse = false;
	my_heap.start -> next = NULL;
	
	printf("Heap Initialized. Overhead per block: %zu bytes\n", sizeof(struct heapchunk_t));
}
//my custom alloc
void *r_alloc(uint32_t size){
	struct heapchunk_t *current = my_heap.start;
	//find a free block, walk thru linked list
	while (current != NULL){
		if(!current -> inuse && current -> size >= size){
			//if we can split, we split
			if(current -> size > size + sizeof(struct heapchunk_t)){
				//initialize new neighbor
				struct heapchunk_t *new_chunk = (struct heapchunk_t *)(
						(char*)current + sizeof(struct heapchunk_t) + size
						);
				new_chunk -> inuse = false;
				new_chunk -> size = current -> size - size - sizeof(struct heapchunk_t);
				new_chunk -> next = current -> next;
				// shrink current block and mark as used
				current -> size = size;
				current -> inuse = true;
				//link to the new neighbor
				current -> next = new_chunk;
				printf("Split! Allocated %u bytes, Remaining: %u \n"
						, size, new_chunk -> size);
			} else {
				//not enough space to split, give all
				current -> inuse = true;
				printf("Taken whole block: %u bytes. \n", current -> size);
			}
			//return pointer to the data
			return (void *)(current + 1);
		}
		//keep looking
		current = current -> next;
	}
	return NULL;
}

void r_free(void *ptr){
	if(ptr == NULL) return;

	struct heapchunk_t *current = (struct heapchunk_t*) ptr - 1;
	current -> inuse =false;
	printf("Freed block at %p (Size: %u)\n", ptr, current->size);

	while(current -> next && current -> next -> inuse == false){
		printf("Coalesce in progress!\n");
		current -> size += sizeof(struct heapchunk_t) + current -> next -> size;
		current -> next = current -> next -> next;
	}
	printf("Block is now %u bytes\n", current -> size);
}

