#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#define BUF_SIZE 4096



int num_procs = 0;
char **proc_ids = NULL;
int num_resources = 0;
char **resource_ids = NULL;
int **max = NULL;
int **allocated = NULL;
int **need = NULL;
int *available = NULL;

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
	
void free_data() {
	for(int i = 0; i < num_procs; i++) {
		free(proc_ids[i]);
	}
	free(proc_ids);
	
	for(int i = 0; i < num_resources; i++) {
		free(resource_ids[i]);
	}
	free(resource_ids);
	
	for(int i = 0; i < num_procs; i++) {
		free(max[i]);
		free(allocated[i]);
		free(need[i]);
	}
	
	free(max);
	free(allocated);
	free(need);
	
	free(available);
}

bool a_lte_b(int a[], int b[], int n) {
	for (int i = 0; i < n; i++) {
		if(a[i] > b[i])
			return false;
	}

	return true;
}

bool print_safety_sequence() {
	int *cur_avail = (int *)calloc(num_resources, sizeof(int));
	memcpy(cur_avail, available, num_resources * sizeof(int));
	printf("Safety sequence: ");
	char safety_sequence[4096] = "";
	bool *finished = (bool *)calloc(num_procs, sizeof(bool));
	int tested = 0;
	int cur_proc = 0;
	int num_finished = 0;
	for (int tested = 0, cur_proc = 0; tested != num_procs && num_finished != num_procs; tested++, cur_proc++, cur_proc %= num_procs)
	{
		if (finished[cur_proc])
			continue;
		if(!a_lte_b(need[cur_proc], cur_avail, num_resources))
			continue;
		tested = 0;
		finished[cur_proc] = true;
		num_finished += 1;
		strcat(strcat(safety_sequence, proc_ids[cur_proc]), " ");
		for (int i = 0; i < num_resources; i++)
		{
			cur_avail[i] += allocated[cur_proc][i];
		}
	}

	bool ret;
	if (num_finished != num_procs)
	{
		printf("No safety sequence exists!\n");
		ret = false;
	} else {
		printf("%s\n", safety_sequence);
		ret = true;
	}

	free(cur_avail);
	free(finished);

	return ret;
}

int main() {
	char buf[4096];
	int offset;
	int temp;
	while (true)
	{
		printf("1. Read Data\n");
		printf("2. Print Data\n");
		printf("3. Find Safe Sequence\n");
		printf("4. Process Request\n");
		printf("5. Exit\n");
		printf("\n");
		int option = atoi(input("Enter an option: "));
		switch(option) {
			case 1:
				free_data();
				char *procs = input("Enter number of process and process ids: ");
				offset = 0;
				sscanf(procs + offset,"%d%n", &num_procs, &temp);
				offset += temp;
				proc_ids = (char **)calloc(num_procs, sizeof(char *));
				for(int i = 0; i < num_procs; i++) {
					sscanf(procs + offset, "%s%n", buf, &temp);
					offset += temp;
					proc_ids[i] = calloc(strlen(buf) + 1, sizeof(char));
					strcpy(proc_ids[i], buf);
				}
				
				char *resources = input("Enter the number of resources, and resource ids: ");
				offset = 0;
				sscanf(resources + offset,"%d%n", &num_resources, &temp);
				offset += temp;
				resource_ids = (char **)calloc(num_resources, sizeof(char *));
				for(int i = 0; i < num_resources; i++) {
					sscanf(resources + offset, "%s%n", buf, &temp);
					offset += temp;
					resource_ids[i] = calloc(strlen(buf) + 1, sizeof(char));
					strcpy(resource_ids[i], buf);
				}
				
				available = (int *)calloc(num_resources, sizeof(int));
				
				char *available_str = input("Enter the number of available instances for each resource type: "); 
				offset = 0;
				
				for(int i = 0; i < num_resources; i++) {
					sscanf(available_str + offset, "%d%n", &available[i], &temp);
					offset += temp;
				}

				max = (int **)calloc(num_procs, sizeof(int *));
				
				for(int i = 0; i < num_procs; i++) {
					max[i] = (int *)calloc(num_resources, sizeof(int));
					char *max_str = input("Enter Max Requirement Vector for %s: ", proc_ids[i]);	
					offset = 0;
					for(int j = 0; j < num_resources; j++) {
						sscanf(max_str + offset, "%d%n", &max[i][j], &temp);
						offset += temp;
					}
				}
				
				allocated = (int **)calloc(num_procs, sizeof(int *));
				
				for(int i = 0; i < num_procs; i++) {
					allocated[i] = (int *)calloc(num_resources, sizeof(int));
					char *alloc_str = input("Enter current Allocation Vector for %s: ", proc_ids[i]);	
					offset = 0;
					for(int j = 0; j < num_resources; j++) {
						sscanf(alloc_str + offset, "%d%n", &allocated[i][j], &temp);
						offset += temp;
					}
				}
				
				need = (int **)calloc(num_procs, sizeof(int *));
				
				for(int i = 0; i < num_procs; i++) {
					need[i] = (int *)calloc(num_resources, sizeof(int));
					for(int j = 0; j < num_resources; j++) {
						need[i][j] = max[i][j] - allocated[i][j];
					}
				}
			break;	
			
			case 2:
				printf("PID\tMAX\tALLOC\tNEED\tAVAIL\n");
				printf("   ");
				for(int i = 0; i < 4; i++) {
					printf("\t");
					for(int j = 0; j < num_resources; j++) {
						printf("%s ", resource_ids[j]);
					}
				}
				printf("\n");
				for(int i = 0; i < num_procs; i++) {
					printf("%s\t", proc_ids[i]);
					for(int j = 0; j < num_resources; j++) {
						printf("%d ", max[i][j]);
					}
					printf("\t");
					for(int j = 0; j < num_resources; j++) {
						printf("%d ", allocated[i][j]);
					}
					printf("\t");
					for(int j = 0; j < num_resources; j++) {
						printf("%d ", need[i][j]);
					}
					printf("\t");
					if(i == 0) {
						for(int j = 0; j < num_resources; j++) {
							printf("%d ", available[j]);
						}
					}
					printf("\n");
				}
			break;
			case 3:
				print_safety_sequence();
			break;
			case 4:
				char *proc_id = input("Enter the process id of the process requesting resources: ");
				int proc_index = -1;
				for(int i = 0; i < num_procs; i++) {
					if(strcmp(proc_ids[i], proc_id) == 0) {
						proc_index = i;
						break;
					}
				}
				int *req_vec = (int *)calloc(num_resources, sizeof(int));
				char *req_vec_str = input("Enter the request vector: ");
				offset = 0;
				for(int i = 0; i < num_resources; i++) {
					sscanf(req_vec_str + offset,"%d%n", &req_vec[i], &temp);
					offset += temp;
				}
				

				
				bool success = true;
				for(int i = 0; i < num_resources; i++) {
					if(available[i] < req_vec[i]) {
						printf("Request for %s is more than available resources, Not granted\n", resource_ids[i]);
						success = false;
						break;
					}
					if(allocated[proc_index][i] + req_vec[i] > max[proc_index][i]) {
						printf("Request for %s is more than Max, Not Granted\n", resource_ids[i]);
						success = false;
						break;
					}
				}
				if(!success)  {
					free(req_vec);
					break;
				}

				for (int i = 0; i < num_resources; i++)
				{
					available[i] -= req_vec[i];
					need[proc_index][i] -= req_vec[i];
					allocated[proc_index][i] += req_vec[i];
				}


				
				if(!print_safety_sequence()) {
					printf("As there is no safety sequence, request not granted.\n");
					for(int i = 0; i < num_resources; i++) {
						available[i] += req_vec[i];
						need[proc_index][i] += req_vec[i];
						allocated[proc_index][i] -= req_vec[i];
					}	
					
				} else {
					printf("As there is a safety sequence, request granted.\n");
				}

				free(req_vec);
			break;
			default:
				free_data();
				printf("Quitting...");
				exit(0);
			break;
		}
	}
	return 0;
}
