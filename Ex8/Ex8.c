#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

typedef struct Node
{
	int start;
	int end;
	int size;
	char *status;
} Node;

Node *node_create(int start, int end, const char *status, Node *n)
{
	n->start = start;
	n->end = end;
	n->size = end - start;
	n->status = (char *)calloc(strlen(status) + 1, sizeof(char));
	strcpy(n->status, status);
	return n;
}

Node *node_init(int start, int end, const char *status)
{
	Node *n = (Node *)malloc(sizeof(Node));
	return node_create(start, end, status, n);
}

#define FAC 2

typedef struct vec_node
{
	int max;
	int size;
	Node **data;
} vec_node;

typedef int (*AllocAlgoType)(vec_node *free_parts, int size);

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

int vec_node_find(vec_node *vec, Node *n, NodeEqualFunc nef)
{
	for (int i = 0; i < vec->size; i++)
	{
		if (nef(vec->data[i], n))
		{
			return i;
		}
	}
	return -1;
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

int comp_func(Node *n1, Node *n2)
{
	if (n1->start == n2->start)
		return 0;
	if (n1->start < n2->start)
		return -1;
	return 1;
}

bool are_nodes_equal(Node *n1, Node *n2)
{
	return strcmp(n1->status, n2->status) == 0;
}

int choose_algo()
{
	printf("Memory Allocation Algorithms: \n");
	printf("1. First Fit\n");
	printf("2. Best Fit\n");
	printf("3. Worst Fit\n");
	int choice = atoi(input("Enter choice: "));
	return choice - 1;
}

int first_fit(vec_node *free_parts, int size)
{
	for (int i = 0; i < vec_node_size(free_parts); i++)
	{
		Node *cur = vec_node_get(free_parts, i);
		if (cur->size >= size)
		{
			return i;
		}
	}
	return -1;
}

int best_fit(vec_node *free_parts, int size)
{
	int best_index = -1;
	int best_size = -1;
	for (int i = 0; i < vec_node_size(free_parts); i++)
	{
		Node *cur = vec_node_get(free_parts, i);
		if (cur->size >= size && (best_index == -1 || cur->size < best_size))
		{
			best_index = i;
			best_size = cur->size;
		}
	}
	return best_index;
}

int worst_fit(vec_node *free_parts, int size)
{
	int worst_index = -1;
	int worst_size = -1;
	for (int i = 0; i < vec_node_size(free_parts); i++)
	{
		Node *cur = vec_node_get(free_parts, i);
		if (cur->size >= size && cur->size > worst_size)
		{
			worst_index = i;
			worst_size = cur->size;
		}
	}
	return worst_index;
}

void print_table(vec_node *elements)
{
	printf("\n");
	if(vec_node_size(elements) == 0) {
		printf("Empty!\n\n");
		return;
	}
	const int CELL_LEN = 10;
	for (int i = 0; i < CELL_LEN * vec_node_size(elements) + 1; i++)
	{
		printf("-");
	}
	printf("\n|");
	for (int i = 0; i < vec_node_size(elements); i++)
	{
		printf(" %-*s|", CELL_LEN - 2, vec_node_get(elements, i)->status);
	}
	printf("\n");
	for (int i = 0; i < CELL_LEN * vec_node_size(elements) + 1; i++)
	{
		printf("-");
	}
	printf("\n%d", vec_node_get(elements, 0)->start);
	for (int i = 0; i < vec_node_size(elements); i++)
	{
		printf("%*d", CELL_LEN, vec_node_get(elements, i)->end);
	}
	printf("\n");
	printf("\n");
}

void display(vec_node *free_parts, vec_node *alloc_parts)
{
	printf("Allocated Memory\n");
	print_table(alloc_parts);
	printf("Free Memory\n");
	print_table(free_parts);
	vec_node *full_table = vec_node_init();
	int i = 0, j = 0;
	while (i + j < vec_node_size(free_parts) + vec_node_size(alloc_parts))
	{
		if (i == vec_node_size(free_parts))
		{
			vec_node_push(full_table, vec_node_get(alloc_parts, j++));
			continue;
		}

		if (j == vec_node_size(alloc_parts))
		{
			vec_node_push(full_table, vec_node_get(free_parts, i++));
			continue;
		}

		Node *f = vec_node_get(free_parts, i);
		Node *a = vec_node_get(alloc_parts, j);
		if (f->start < a->start)
		{
			vec_node_push(full_table, f);
			i++;
		}
		else
		{
			vec_node_push(full_table, a);
			j++;
		}
	}
	printf("Full Memory\n");
	print_table(full_table);
	vec_node_free(full_table, false);
}

void coalesce(vec_node *free_parts)
{
	for (int i = 0; i < vec_node_size(free_parts) - 1; i++)
	{
		Node *cur = vec_node_get(free_parts, i);
		Node *next = vec_node_get(free_parts, i + 1);
		if (cur->end == next->start)
		{
			cur->end = next->end;
			cur->size += next->size;
			vec_node_delete(free_parts, i + 1);
			i -= 1;
		}
	}
}

int main()
{
	AllocAlgoType alloc_algos[3] = {first_fit, best_fit, worst_fit};

	int num_partitions = atoi(input("Enter the number of partitons in memory: "));
	vec_node *free_parts = vec_node_init();
	vec_node *alloc_parts = vec_node_init();
	for (int i = 0; i < num_partitions; i++)
	{
		int start, end;
		sscanf(input("Enter the starting and ending address of partition %d: ", i + 1), "%d %d", &start, &end);
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
		switch (option)
		{
		case 1:

			strcpy(proc_id, input("Enter process id: "));
			int size = atoi(input("Enter size needed: "));
			int free_index = alloc_algo(free_parts, size);
			if (free_index == -1)
			{
				printf("No space available to allocate memory!\n");
				break;
			}
			else
			{
				printf("Allocation success!\n");
				Node *cur = vec_node_get(free_parts, free_index);
				int alloc_index = -vec_node_search(alloc_parts, cur, comp_func) - 1;
				vec_node_insert(alloc_parts, alloc_index, node_init(cur->start, cur->start + size, proc_id));

				if (cur->size == size)
				{
					vec_node_delete(free_parts, free_index);
				}
				else
				{
					cur->size -= size;
					cur->start = cur->start + size;
				}
			}
			display(free_parts, alloc_parts);
			break;
		case 2:
			Node n;
			node_create(0, 0, input("Enter process id: "), &n);
			for (int i = 0; true; i++)
			{
				int index = vec_node_find(alloc_parts, &n, are_nodes_equal);
				if (index == -1)
				{
					if(i > 0) {
						printf("Blocks Deleted: %d\n", i);
					} else {
						printf("Error! No such process exists!\n");
					}
					break;
				}

				Node *del = vec_node_get(alloc_parts, index);
				int actual_index = -vec_node_search(free_parts, del, comp_func) - 1;
				vec_node_insert(free_parts, actual_index, node_init(del->start, del->end, "H"));
				vec_node_delete(alloc_parts, index);
			}
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
			vec_node_free(free_parts, true);
			vec_node_free(alloc_parts, true);
			exit(0);
			break;
		}
	}
	return 0;
}
