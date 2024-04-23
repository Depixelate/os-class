#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

#define NAME_SIZE 100
#define FILE_SIZE 4096

typedef enum Status
{
	WAITING,
	SUCCESS,
	ERROR
} Status;

typedef struct SharedMem
{
	Status client_status;
	Status server_status;
	char filename[NAME_SIZE];
	int size;
	char file[FILE_SIZE];
} SharedMem;

int main() {
	key_t key = ftok("/tmp", 65);
	if(key < 0)
		perrorc("An error occurred while generating the key");
	int id = shmget(key, sizeof(SharedMem), IPC_CREAT | 0777);
	if (id < 0) perrorc("An error occurred while creating the shared memory segment");
	void *temp = shmat(id, NULL, 0);
	if(temp == (void *)-1) perrorc("An error occurred while attaching to the shared memory segment");
	SharedMem *shm = (SharedMem *)temp;
	shm->client_status = WAITING;
	shm->server_status = WAITING;
	printf("Enter file path: ");
	fgets(shm->filename, NAME_SIZE, stdin);
	shm->filename[strlen(shm->filename)-1] = '\0';
	shm->client_status = SUCCESS;
	while (shm->server_status == WAITING)
		;

	if (shm->server_status == ERROR) {
		fprintf(stderr, "The following error occurred in the server: %s", shm->file);
		exit(1);
	}

	char copy_file_name[NAME_SIZE] = "";
	char *dot_location = strstr(shm->filename, ".");
	strncat(copy_file_name, shm->filename, dot_location != NULL ? dot_location - shm->filename : strlen(shm->filename));
	strcat(strcat(copy_file_name, "-copy"), dot_location != NULL ? dot_location : "");
	int fd = open(copy_file_name, O_CREAT | O_WRONLY, 0777);
	if(fd < 0)
		perrorc("The following error occurred while trying to open the file copy");

	printf("File Content: \n");
	for(int i = 0; i < shm->size; i+=0) {
		int chars_written = write(fd, shm->file + i, 1);
		if (chars_written < 0)
			perrorc("The following error occurred while writing to the file: ");
		if (i != 0) putchar(shm->file[i]);
		i += chars_written;
	}

	shm->client_status = WAITING;

	close(fd);
	int result = shmdt(shm);
	if (result < 0)
		perrorc("An error occurred while detaching from the shared memory segment");
	if (shmctl(id, IPC_RMID, NULL) < 0)
		perrorc("An error occurred when deleting the memory segment");
}
