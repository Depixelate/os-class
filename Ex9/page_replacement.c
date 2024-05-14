#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define BUF_SIZE 4096

char *input(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	static char buf[4096];
	fgets(buf, BUF_SIZE, stdin);
	buf[strlen(buf) - 1] = '\0';
	return buf;
}

/* Node stuff, modify */

typedef struct Node
{
	int page;
} Node;

Node *node_create(int page, Node *n)
{
	n->page = page;
	return n;
}

Node *node_init(int page)
{
	Node *n = (Node *)malloc(sizeof(Node));
	return node_create(page, n);
}

Node *node_free(Node *n) {
	free(n);
}

int comp_func(Node *n1, Node *n2)
{
	if (n1->page == n2->page)
		return 0;
	if (n1->page < n2->page)
		return -1;
	return 1;
}

bool node_equal_func(Node *n1, Node *n2)
{
	return n1->page == n2->page;
}
/* Node stuff, modify */

#define FAC 2

typedef struct vec_node
{
	int max;
	int size;
	Node **data;
} vec_node;

typedef bool (*NodeEqualFunc)(Node *n1, Node *n2);

typedef int (*CompFunc)(Node *n1, Node *n2);

vec_node *vec_node_init()
{
	vec_node *v = (vec_node *)malloc(sizeof(vec_node));
	v->max = 1;
	v->size = 0;
	v->data = (Node **)calloc(v->max, sizeof(Node *));
	return v;
}

void vec_node_free(vec_node *vec, bool free_items)
{
	if(free_items) {
		for (int i = 0; i < vec->size; i++) {
			free(vec->data[i]);
		}
	}
	free(vec->data);
	free(vec);
}

int vec_node_size(vec_node *vec)
{
	return vec->size;
}

Node *vec_node_get(vec_node *vec, int index)
{
	return vec->data[index];
}

void resize_if_full(vec_node *vec)
{
	if (vec->size == vec->max)
	{
		vec->max *= FAC;
		vec->data = realloc(vec->data, vec->max * sizeof(Node *));
	}
}

void vec_node_insert(vec_node *vec, int index, Node *n)
{
	resize_if_full(vec);
	for (int i = vec->size - 1; i >= index; i--)
	{
		vec->data[i + 1] = vec->data[i];
	}
	vec->data[index] = n;
	vec->size += 1;
}

void vec_node_push(vec_node *vec, Node *n)
{
	vec_node_insert(vec, vec->size, n);
}

void vec_node_delete(vec_node *vec, int index)
{
	free(vec->data[index]);
	for (int i = index; i < vec->size - 1; i++)
	{
		vec->data[i] = vec->data[i + 1];
	}
	vec->size -= 1;
}

int vec_node_find(vec_node *vec, Node *n, int start, int end, NodeEqualFunc nef)
{
	for (int i = start; i < end; i++)
	{
		if (nef(vec->data[i], n))
		{
			return i;
		}
	}
	return -1;
}

int vec_node_find_c(vec_node *vec, Node *n, NodeEqualFunc nef)
{
	return vec_node_find(vec, n, 0, vec->size, nef);
}

int vec_node_find_rev(vec_node *vec, Node *n, int start, int end, NodeEqualFunc nef)
{
	for (int i = end - 1; i >= start; i--)
	{
		if (nef(vec->data[i], n))
		{
			return i;
		}
	}
	return -1;
}

int vec_node_find_rev_c(vec_node *vec, Node *n, NodeEqualFunc nef)
{
	return vec_node_find_rev(vec, n, 0, vec->size, nef);
}

int vec_node_count(vec_node *vec, Node *n, int start, int end, NodeEqualFunc nef) {
	int count = 0;
	for(int i = start; i < end; i++) {
		if(nef(vec->data[i], n)) {
			count += 1;
		}
	}
	return count;
}

int vec_node_count_c(vec_node *vec, Node *n, NodeEqualFunc nef) {
	return vec_node_count(vec, n, 0, vec->size, nef);
}

int vec_node_search(vec_node *vec, Node *n, CompFunc cf)
{
	int start = 0;
	int end = vec->size;
	while (start != end)
	{
		int mid = (start + end) / 2;
		Node *test = vec->data[mid];
		int res = cf(n, test);
		if (res == 0)
		{
			return mid;
		}
		else if (res < 0)
		{
			end = mid;
		}
		else
		{
			start = mid + 1;
		}
	}
	return -start - 1;
}


int fifo(vec_node *alloc_seq, vec_node *frames, int cur_index) {
	static int first_index = 0;
	int temp = first_index;
	first_index += 1;
	first_index %= frames->size;
	return temp;	
}

int optimal(vec_node *alloc_seq, vec_node *frames, int cur_index) {
	int max_time = -1;
	int max_index = -1;
	for(int i = 0; i < frames->size; i++) {
		Node n;
		int time = vec_node_find(alloc_seq, node_create(frames->data[i]->page, &n), cur_index + 1, alloc_seq->size, node_equal_func);
		if (time == -1) time = alloc_seq->size;
		if(time > max_time) {
			max_time = time;
			max_index = i;
		}
	}
	return max_index;
}

int lru(vec_node *alloc_seq, vec_node *frames, int cur_index) {
	int min_time = alloc_seq->size;
	int min_index = -1;
	for(int i = 0; i < frames->size; i++) {
		Node n;
		int time = vec_node_find_rev(alloc_seq, node_create(frames->data[i]->page, &n), 0, cur_index, node_equal_func);
		assert(time >= 0);
		if(time < min_time) {
			min_time = time;
			min_index = i;
		}
	}
	return min_index;
}

int lfu(vec_node *alloc_seq, vec_node *frames, int cur_index) {
	int min_count = alloc_seq->size;
	int min_index = -1;
	for(int i = 0; i < frames->size; i++) {
		Node n;
		int count = vec_node_count(alloc_seq, node_create(frames->data[i]->page, &n), 0, cur_index, node_equal_func);
		assert(count > 0);
		if(count < min_count) {
			min_count = count;
			min_index = i;
		}
	}
	return min_index;
}

typedef int (*ReplaceAlgo)(vec_node *alloc_seq, vec_node *frames, int cur_index);

int main() {
	char buf[BUF_SIZE];
	int num_pages = atoi(input("Enter the number of pages: "));
	int num_frames = atoi(input("Enter the number of frames: "));
	strcpy(buf, input("Enter the allocation string: "));
	vec_node *alloc_seq = vec_node_init();
	vec_node *frames = vec_node_init();
	int offset = 0;
	for(int i = 0; true; i++) {
		int page;
		int temp;
		int res = sscanf(buf + offset, "%d%n", &page, &temp);
		if(res == EOF) break;
		offset += temp;
		vec_node_push(alloc_seq, node_init(page));
	}
	
	ReplaceAlgo algos[] = {fifo, optimal, lru, lfu};
	printf("Replacement Algorithms: \n");
	printf("1. FIFO\n");
	printf("2. Optimal\n");
	printf("3. LRU\n");
	printf("4. LFU\n");
	ReplaceAlgo algo = algos[atoi(input("Enter the desired option: ")) - 1];
	
	int page_faults = 0;
	
	printf("Allocation Sequence\n\n");
	
	for(int i = 0; i < alloc_seq->size; i++) {
		int page = alloc_seq->data[i]->page;
		printf("%d\t", page);
		if(vec_node_find_c(frames, alloc_seq->data[i], node_equal_func) != -1) {
			printf("\n");
			continue;
		}
		if(frames->size < num_frames) {
			vec_node_push(frames, node_init(page));
		} else {
			int replace_index = algo(alloc_seq, frames, i);
			frames->data[replace_index]->page = page;
		}
		
		for(int i = 0; i < frames->size; i++) {
			printf("%d ", frames->data[i]->page);
		}
		
		printf("\n");
		
		page_faults += 1;
	}
	
	printf("\n");
	
	printf("Total number of page faults: %d\n", page_faults);
	
	return 0;
}
