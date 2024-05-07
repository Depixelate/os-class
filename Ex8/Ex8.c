#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#define BUF_SIZE 4096

char *input(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	static char buf[4096];
	fgets(buf, BUF_SIZE, stdin);
	buf[strlen(buf) - 1] = '\0';
	return buf;
}

typedef struct Node {
	int start;
	int end;
	int size;
	char *status;
} Node;

typedef int (*AllocAlgoType)(vec_node *free_parts, int size);	

typedef bool (*NodeEqualFunc)(Node *n1, Node *n2);

typedef int (*CompFunc)(Node *n1, Node *n2);

int comp_func(Node *n1, Node *n2) {
	if(n1->start == n2->start) return 0;
	if(n1->start < n2->start) return -1;
	return 1;
}

bool are_nodes_equal(Node *n1, Node *n2) {
	return strcmp(n1->status, n2->status) == 0;	
}

int choose_algo() {
	printf("Memory Allocation Algorithms: \n");
	printf("1. First Fit\n");
	printf("2. Best Fit\n");
	printf("3. Worst Fit\n");
	int choice = atoi(input("Enter choice: "));
	return choice - 1;
}

bool first_fit(vec_node *free_parts, vec_node *alloc_parts, const char *pid, int size) {
	for(int i = 0; i < vec_node_size(free_parts); i++) {
		Node *cur = vec_node_get(free_parts, i);
		if(cur->size >= size) {
			return i;
		}
	}
	return -1;
}

int best_fit(vec_node *free_parts, int size) {
	int best_index = -1;
	int best_size = -1;
	for(int i = 0; i < vec_node_size(free_parts); i++) {
		Node *cur = vec_node_get(free_parts, i);
		if(cur->size >= size && (best_index == -1 || cur->size < best_size)) {
			best_index = i;
			best_size = cur->size;
		}
	}
	return best_index;
}

int worst_fit(vec_node *free_parts, int size) {
	int worst_index = -1;
	int worst_size = -1;
	for(int i = 0; i < vec_node_size(free_parts); i++) {
		Node *cur = vec_node_get(free_parts, i);
		if(cur->size >= size && cur->size > worst_size) {
			worst_index = i;
			worst_size = cur->size;
		}
	}
	return worst_index;
}

void display(vec_node *free_parts, vec_node *alloc_parts) {
	
}

void coalesce(vec_node *free_parts) {
	for(int i = 0; i < vec_node_size(free_parts) - 1; i++) {
		Node *cur = vec_node_get(free_parts, i);
		Node *next = vec_node_get(free_parts, i + 1);
		if(cur->end == next->start) {
			for(int j = 0;
		}
	}
}

int main() {
	AllocAlgoType alloc_algos[3] = {first_fit, best_fit, worst_fit};
	
	int num_partitons = atoi(input("Enter the number of partitons in memory: "));
	vec_node *free_parts = vec_node_init( num_partitons);
	vec_node *alloc_parts = vec_node_init(0);
	for(int i = 0; i < num_partitions; i++) {
		int start, end;
		sscanf(input("Enter the starting and ending address of partition %d", i + 1), "%d %d", &start, &end);
		vec_node_push(free_parts, node_init(start, end, "H"));
	}
	
	display(free_parts, alloc_parts);
	
	AllocAlgoType alloc_algo;
	alloc_algo = alloc_algos[choose_algo()];  
	
	while (true)
	{
		printf("1. Entry/Allocate\n");
		printf("2. Exit / Deallocate\n");
		printf("3. Display\n");
		printf("4. Coalescing of Holes\n");
		printf("5. Choose Algorithm\n");
		printf("6. Exit\n");
		printf("\n");
		int option = atoi(input("Enter an option: "));
		char proc_id[4096];
		switch(option) {
			case 1:

				strcpy(proc_id, input("Enter process id: "));
				int size = atoi(input("Enter size needed: ");
				int free_index = alloc_algo(free_parts, size);
				if(free_index == -1) {
					printf("No space available to allocate memory!\n");
					break;
				} else {
					printf("Allocation success!\n");
					Node *cur = vec_node_get(free_parts, free_index);
					int alloc_index = -vec_node_search(alloc_parts, cur, comp_func) - 1;
					vec_node_insert(alloc_parts, alloc_index, node_init(cur->start, cur->start + size, proc_id));
		
					if(cur->size == size) {
						vec_node_delete(free_parts, free_index);
					} else {
						cur->size -= size;
						cur->start = cur->start + size;
					}
				}
				display(free_parts, alloc_parts);
			break;
			case 2:
				Node n;
				node_init(0, 0, input("Enter process id: "));
				int index = vec_node_find(alloc_parts, n, are_nodes_equal);
				if(index == -1) {
					printf("Error!, No such process exists!\n")'
					break;
				}
				
				Node *del = vec_node_get(alloc_parts, index);
				int actual_index = -vec_node_search(free_parts, del, comp_func) - 1;
				vec_node_insert(free_parts, actual_index, node_init(del->start, del->end, "H"));
				vec_node_delete(alloc_parts, index);
				display(free_parts, alloc_parts);
			break;		
			
			case 3:
				display(free_parts, alloc_parts);
			break;
			case 4:
				coalesce(free_parts);
				display(free_parts, alloc_parts);
			break;
			case 5:
				alloc_algo = alloc_algos[choose_algo()];
			break;
			default:
				vec_node_free(free_parts);
				vec_node_free(alloc_parts);
				exit(0);
			break;
	}
	return 0;
}
