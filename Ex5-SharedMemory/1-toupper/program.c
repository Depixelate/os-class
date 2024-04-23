#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>


void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

#define BUF_SIZE 4096

typedef struct SharedMem {
	bool client_ready;
	char string[BUF_SIZE];
} SharedMem;

int main() {
	int id = shmget(IPC_PRIVATE, sizeof(SharedMem), 0777);
	if (id < 0) perrorc("An error occurred while creating the shared memory segment");
	int pid = fork();
	if (pid < 0) perrorc("An error occurred while forking the process");
	SharedMem *shared_mem = (SharedMem *)shmat(id, NULL, 0);
	
	if ((int)shared_mem == -1)
		perrorc("An error occurred while attaching to the shared memory segment");

	shared_mem->client_ready = false;

	if (pid != 0) {
		printf("Parent: Enter a name to convert to uppercase: ");
		fgets(shared_mem->string, BUF_SIZE, stdin);
		shared_mem->string[strlen(shared_mem->string)-1] = '\0';
		shared_mem->client_ready = true;
	}
	else
	{
		while(!shared_mem->client_ready)
			;
		char temp[BUF_SIZE];
		strcpy(temp, shared_mem->string);
		for(int i = 0; temp[i] != '\0'; i++) {
			temp[i] = toupper(temp[i]);
		}
		printf("Child: Name in uppercase: %s\n", temp);
	}
	int result = shmdt(shared_mem);
	if (result < 0)
		perrorc("An error occurred while detaching from the shared memory segment");
	if(pid != 0) {
		if (shmctl(id, IPC_RMID, NULL) < 0)
			perrorc("An error occurred when deleting the memory segment");
	}
}
